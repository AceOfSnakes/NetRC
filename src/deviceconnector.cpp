/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QFileDialog>
#include <QDebug>
#include <QImage>
#include <QMessageBox>
#include <QGroupBox>
#include <QNetworkInterface>
#include "deviceconnector.h"
#include "autosearchdialog.h"
#include "filedialogwithhistory.h"
#include "ui_deviceconnector.h"
#include "rcsettings.h"
#include "autosearchdialog.h"

DeviceConnector::DeviceConnector(QVariant &sets, QWidget *parent) :
    QDialog(parent),
    reIP("[.]"),
    settings(sets),
    ui(new Ui::DeviceConnector)
{
    ui->setupUi(this);

    QString family = settings.toMap().value("family").toString();
    setWindowTitle(qApp->applicationName().append(". Connect to \"").append(family).append("\""));

    ui->knownDevicesComboBox->addItems(RCSettings::devicesList());


    if(!family.isEmpty()) {
        ui->deviceProtocol->addItem(family);
    }
    if(settings.toMap().value("ping"
                               "").toString().isEmpty()) {
        ui->groupBoxConnect->setEnabled(false);
        ui->deviceProtocol->setEnabled(true);
    }
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);

    for (auto &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            QStringList l = address.toString().split(reIP, Qt::SkipEmptyParts);
            if (l.size() == 4) {
                setIpAddress(l[0], l[1], l[2], l[3], "?");
                break;
            }
        }
    }
    connect(ui->autoSearchButton, SIGNAL(clicked()), this, SLOT(autoSearchClicked()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(applyButtonClicked()));
    connect(ui->deleteConfig, SIGNAL(clicked()), this, SLOT(deleteFamilyClicked()));
    connect(ui->loadConfig, SIGNAL(clicked()), this, SLOT(loadConfigClicked()));
    connect(ui->deviceProtocol, SIGNAL(currentTextChanged(QString)),
            this, SLOT(deviceProtocolCurrentIndexChanged(QString)));
    connect(ui->knownDevicesComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onKnownDevicesComboBoxCurrentIndexChanged(int)));
    connect(ui->uploadLogo, SIGNAL(clicked(bool)),
            this, SLOT(onLogoUploaded(bool)));
    connect(ui->removeLogo, SIGNAL(clicked(bool)),
            this, SLOT(onLogoRemoved(bool)));
    connect(ui->removeKnownDevice, SIGNAL(clicked(bool)),
            this, SLOT(onRemoveKnownDevice()));
    reloadDevicesFamily();

}

DeviceConnector::~DeviceConnector() {
    delete ui;
}

void DeviceConnector::setDevice(QString deviceFamily, QString device,
                                QString address, unsigned int port,
                                QPixmap logo) {

    ui->deviceProtocol->setCurrentText(deviceFamily);
    ui->line_DeviceName->setText(device);

    if(!ui->line_DeviceName->text().isEmpty()) {
        ui->knownDevicesComboBox->setCurrentText(ui->line_DeviceName->text());
    }
    QStringList l = address.split(reIP, Qt::SkipEmptyParts);
    if (l.size() == 4) {
        if(port == 0) {
            setIpAddress(l[0], l[1], l[2], l[3], "?");
        } else {
            setIpAddress(l[0], l[1], l[2], l[3], QString().asprintf("%d", port));
        }
    }
    img = logo.copy();
    loadLogo();
}

void DeviceConnector::autoSearchClicked() {
    AutoSearchDialog *autoSearchDialog = nullptr;
    do {
        delete autoSearchDialog;
        autoSearchDialog = new AutoSearchDialog(this,
                                                settings.toMap().value("pingCommands").toList().at(0).toString(),
                                                settings.toMap().value("pingResponseOk").toString(),
                                                settings.toMap().value("pingResponseErr").toString(),
                                                settings.toMap().value("prefferedPort").toInt());
        autoSearchDialog->setWindowTitle(qApp->applicationName()
                                         .append(". Auto Search \"")
                                         .append(settings.toMap().value("family").toString()
                                                 .append("\"")));
        autoSearchDialog->exec();
    } while(autoSearchDialog->result == 2);

    if(autoSearchDialog->result == 1) {
        QString ip = autoSearchDialog->selectedAddress;
        int port = autoSearchDialog->selectedPort;
        QStringList l = ip.split(reIP, Qt::SkipEmptyParts);
        if (l.size() == 4) {
            setIpAddress(l[0], l[1], l[2], l[3], QString("%1").arg(port));
            deviceAddress = QString("%1.%2.%3.%4:%5").arg(l[0], l[1], l[2], l[3]).arg(port);
            deviceIPAddress = QString("%1.%2.%3.%4").arg(l[0], l[1], l[2], l[3]);
            devicePort = port;
        }

        device = autoSearchDialog->selectedDevice;
        deviceFamily = settings.toMap().value("family").toString();
        ui->line_DeviceName->setText(autoSearchDialog->selectedDevice);
    }
    delete autoSearchDialog;
}
QString DeviceConnector::getIpAddress() {
    return ui->lineEditIP1->text().append(".").append(ui->lineEditIP2->text()).append(".")
            .append(ui->lineEditIP3->text()).append(".").append(ui->lineEditIP4->text());
}

void DeviceConnector::setIpAddress(QString ip1, QString ip2, QString ip3, QString ip4, QString port) {
    ui->lineEditIP1->setText(ip1);
    ui->lineEditIP2->setText(ip2);
    ui->lineEditIP3->setText(ip3);
    ui->lineEditIP4->setText(ip4);
    ui->lineEditIPPort->setText(port);
}

void DeviceConnector::applyButtonClicked()
{
    QString addr = getIpAddress();

    qDebug() << "Password" << cryptoSettings.key.password;
    qDebug() << "cipher" << cryptoSettings.cipher;

    deviceIPAddress = addr;
    deviceAddress = addr.append(":").append(ui->lineEditIPPort->text());
    device = ui->line_DeviceName->text();
    devicePort = ui->lineEditIPPort->text().toInt();
    deviceFamily = settings.toMap().value("family").toString();
    if(ui->rememberPermanently->isChecked()) {
        QSettings sets(qApp->organizationName(),qApp->applicationName());
        sets.beginGroup("global");
        sets.beginGroup("devices");
        sets.beginGroup(QString(device).replace("/","@@"));
        sets.setValue("deviceName", device);
        sets.setValue("deviceFamily", deviceFamily);
        sets.setValue("devicePort", devicePort);
        sets.setValue("deviceIPAddress", deviceIPAddress);
        if(!img.isNull()) {
            sets.setValue("deviceLogo", img.toImage());
        }
        else {
            sets.remove("deviceLogo");
        }
        sets.endGroup();
        sets.endGroup();
        sets.endGroup();
    }
    close();
}

void DeviceConnector::loadConfigClicked() {

    QString xfile = FileDialogWithHistory().resolveLoadFileName(this, tr("Load RC Settings Protocol"),
                             tr("JSON Files")
                             .append(" (*.json);;")
                             .append(tr("All files"))
                             .append(" (* *.*)"), "config");
    if (!xfile.isEmpty()) {
        QFile file(xfile);
        QVariant var = RCSettings::load(file);
        select(var);

        reloadDevicesFamily();
        deviceProtocolCurrentIndexChanged(var.toMap().value("family").toString());
    }
}

void DeviceConnector::select(QVariant vars) {
    settings.swap(vars);
    ui->groupBoxConnect->setEnabled(true);
    QString family = settings.toMap().value("family").toString();
    deviceFamily = family;
    setWindowTitle(qApp->applicationName().append(". Connect to \"").append(family).append("\""));
}

void DeviceConnector::reloadDevicesFamily() {
    onLogoRemoved(true);
    QStringList saved = RCSettings::settingsList();
    ui->knownDevicesComboBox->clear();
    ui->knownDevicesComboBox->addItems(RCSettings::devicesList());
    saved.sort();
    ui->deviceProtocol->clear();
    ui->deviceProtocol->addItems(saved);
    QString family = settings.toMap().value("family").toString();
    if(ui->deviceProtocol->findText(family) > 0) {
        ui->deviceProtocol->setCurrentIndex(ui->deviceProtocol->findText(family));
    }
    reloadCryptoSettings();
}

void DeviceConnector::applyCryptoWidgetStyle(QWidget * widget, bool editable) {
    QFont font = widget->font();
    font.setFamily("Courier New");
    font.setPointSize(9);
    font.setStyleHint(QFont::Monospace);
    widget->setFont(font);
    if (!editable) {
        widget->setEnabled(false);
    }
}

QWidget * DeviceConnector::createCryptoWidget(QString & value, bool editable) {
    QWidget *ret = new QLineEdit(value);
    applyCryptoWidgetStyle(ret, editable);
    return ret;
}

QWidget * DeviceConnector::createCryptoWidget(int & value, bool editable) {
    QWidget *ret = new QLineEdit(QString::number(value));
    applyCryptoWidgetStyle(ret, editable);
    return ret;
}

QWidget * DeviceConnector::createCryptoWidget(QByteArray value, bool editable) {
    QWidget *ret = new QLineEdit(value);
    applyCryptoWidgetStyle(ret, editable);
    return ret;
}

void DeviceConnector::applyCryptoToUI(QHash<QString, QVariant> crypto, QGridLayout * layout) {
    qDebug() << "" << cryptoSettings.key.password;
    int idx = 0;
    layout->setColumnMinimumWidth(0, 102);
    layout->addWidget(new QLabel(" Cipher"), idx, 0);
    layout->addWidget(createCryptoWidget(cryptoSettings.cipher), idx++, 1);

    layout->addWidget(new QLabel(" Padding"), idx, 0);
    layout->addWidget(createCryptoWidget(cryptoSettings.padding), idx++, 1);

    QGroupBox *inner = new QGroupBox("Key", this);
    QGridLayout *lay = new QGridLayout();
    inner->setLayout(lay);

    lay->setColumnMinimumWidth(0, 90);
    layout->addWidget(
         inner,
         idx++, 0, 1, 2);
    int layidx = 0;

    lay->addWidget(new QLabel("Password Type"), layidx, 0);
    QComboBox * passType = new QComboBox();
    QMetaEnum metaEnum = QMetaEnum::fromType<Crypto::PasswordType>();
    for(int i = 0; i < metaEnum.keyCount(); i++) {
        passType->addItem(QString().asprintf("%d", i));
    }
//    passType->setEnabled(false);
    lay->addWidget(passType, layidx++, 1);

    lay->addWidget(new QLabel("Password ***"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.key.password, true), layidx++, 1);

    lay->addWidget(new QLabel("Code iterations"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.key.iterations), layidx++, 1);

    lay->addWidget(new QLabel("Size (bits)"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.key.bitsSize), layidx++, 1);

    lay->addWidget(new QLabel("Salt"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.key.salt.toHex(' ').toUpper()), layidx++, 1);


    inner = new QGroupBox("IV (Initialization vector)", this);
    lay = new QGridLayout();

    inner->setLayout(lay);
    // valueLabel->setMinimumWidth(100);
    lay->setColumnMinimumWidth(0, 90);
    layout->addWidget(
        inner,
        idx, 0, 1, 2);

    layidx = 0;

    lay->addWidget(new QLabel("IV Type"), layidx, 0);

    QComboBox * ivType = new QComboBox();
    metaEnum = QMetaEnum::fromType<Crypto::IVType>();
    qDebug() << "metaEnum.keyCount()" << metaEnum.keyCount();
    for(int i = 0; i < metaEnum.keyCount(); i++) {
        ivType->addItem(QString().asprintf("%d",i));
    }
    lay->addWidget(ivType, layidx++, 1);

    lay->addWidget(new QLabel("Size (bits)"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.iv.bitsSize), layidx++, 1);

    lay->addWidget(new QLabel("Cipher"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.iv.cipher), layidx++, 1);

    lay->addWidget(new QLabel("Value"), layidx, 0);
    lay->addWidget(createCryptoWidget(cryptoSettings.iv.value.toHex(' ').toUpper()), layidx++, 1);
}

void DeviceConnector::reloadCryptoSettings() {

    QVariant crypto = settings.toHash().value("crypto");
    //crypto.to
    QLayoutItem *child;
    while ((child = ui->cryptoGridLayout->takeAt(0)) != nullptr) {
        child->widget()->hide();
        delete child;
    }

    if(crypto.isValid()) {
        applyCryptoToUI(crypto.toHash(), ui->cryptoGridLayout);
        ui->cryptoBox->setVisible(true);
    } else {
        ui->cryptoBox->setVisible(false);
    }

    ui->cryptoLine->setVisible(crypto.isValid());

    adjustSize();
    setMaximumSize(minimumSize());
    repaint();
}

void DeviceConnector::deviceProtocolCurrentIndexChanged(const QString &arg1) {
    if(!arg1.isEmpty()) {
        select(RCSettings::load(arg1));
        ui->lineEditIPPort->setText(settings.toMap().value("prefferedPort").toString());
        ui->line_DeviceName->setText("");

        ui->lineEditIP4->setText("?");
        if(ui->deviceProtocol->findText(arg1) > 0) {
            ui->deviceProtocol->setCurrentIndex(ui->deviceProtocol->findText(arg1));
        }
        onLogoRemoved(true);
        reloadCryptoSettings();
    }
}

void DeviceConnector::deleteFamilyClicked() {
    if(!ui->deviceProtocol->currentText().isEmpty()) {
        QMessageBox::StandardButton button = QMessageBox::information(
                    this,
                    QString("Remove \"").append(ui->deviceProtocol->currentText()).append("\""),
                    QString("Remove \"").append(ui->deviceProtocol->currentText()).append("\" from available device protocols?    ")
                    .append("\nAll permanently saved assosiated devices will also be deleted    \n\n\n"),
                    QMessageBox::Yes | QMessageBox::No
                    );
        if(button == QMessageBox::Yes) {
            RCSettings::remove(ui->deviceProtocol->currentText());
            reloadDevicesFamily();
        }
    }
}

void DeviceConnector::onKnownDevicesComboBoxCurrentIndexChanged(int index) {
    QVariant sets = RCSettings::deviceSettings(ui->knownDevicesComboBox->itemText(index));
    onLogoRemoved(true);
    QImage image = sets.toMap().value("deviceLogo").value<QImage>();
    img = QPixmap::fromImage(image);
    if (sets.isValid()) {
        setDevice(sets.toMap().value("deviceFamily", "").toString(),
                sets.toMap().value("deviceName", "").toString(),
                sets.toMap().value("deviceIPAddress", "").toString(),
                sets.toMap().value("devicePort").toUInt(),
                img);
        loadLogo();
    }
}

void DeviceConnector::onRemoveKnownDevice() {
   RCSettings::removeDevice(QString(ui->knownDevicesComboBox->currentText()));
   ui->knownDevicesComboBox->clear();
   ui->knownDevicesComboBox->addItems(RCSettings::devicesList());
}

void DeviceConnector::loadLogo()
{
    if(!img.isNull()) {
        ui->logoLabel->setText("");
        img = img.scaledToHeight(16);
        ui->logoLabel->setPixmap(img);
    }
    else {
        ui->logoLabel->setText("NetRC");
    }
}

void DeviceConnector::onLogoUploaded(bool) {
    QString xfile = FileDialogWithHistory().resolveLoadFileName(this, tr("Load Logo"),
                                                                tr("Image files")
                                                                    .append(" (*.png);;")
                                                                    .append(tr("All files"))
                                                                    .append(" (* *.*)"), "logo");
    if (!xfile.isEmpty()) {
        QFile file(xfile);
        img.load(file.fileName());
        loadLogo();
    }
}

void DeviceConnector::onLogoRemoved(bool) {
    img = QPixmap();
    img.loadFromData(nullptr);
    ui->logoLabel->setPixmap(img);
    ui->logoLabel->setText("NetRC");
}

