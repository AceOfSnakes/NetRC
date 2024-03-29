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
#include <QMessageBox>
#include <QNetworkInterface>
#include "deviceconnector.h"
#include "autosearchdialog.h"
#include "filedialogwithhistory.h"
#include "ui_deviceconnector.h"
#include "rcsettings.h"
#include "autosearchdialog.h"

DeviceConnector::DeviceConnector(QVariant &sets, QWidget *parent) :
    QDialog(parent),
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
    if(settings.toMap().value("pingResponseOk").toString().isEmpty()) {
        ui->groupBox_Connect->setEnabled(false);
        ui->deviceProtocol->setEnabled(true);
    }
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            QStringList l = address.toString().split(QRegularExpression("[.]"), Qt::SkipEmptyParts);
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
    connect(ui->deviceProtocol, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(deviceProtocolCurrentIndexChanged(const QString &)));
    connect(ui->knownDevicesComboBox,SIGNAL(currentIndexChanged(int)),
            this, SLOT(onKnownDevicesComboBoxCurrentIndexChanged(int)));
    reloadDevicesFamily();
}

DeviceConnector::~DeviceConnector() {
    delete ui;
}

void DeviceConnector::setDevice(QString deviceFamily, QString device, QString address, unsigned int port) {
    ui->deviceProtocol->setCurrentText(deviceFamily);
    ui->line_DeviceName->setText(device);

    if(!ui->line_DeviceName->text().isEmpty()) {
        ui->knownDevicesComboBox->setCurrentText(ui->line_DeviceName->text());
    }

    QStringList l = address.split(QRegularExpression("[.]"), Qt::SkipEmptyParts);
    if (l.size() == 4) {
        if(port == 0) {
            setIpAddress(l[0], l[1], l[2], l[3], "?");
        } else {
            setIpAddress(l[0], l[1], l[2], l[3], QString().asprintf("%d", port));
        }
    }
}

void DeviceConnector::autoSearchClicked() {
    AutoSearchDialog *autoSearchDialog = nullptr;
    do {
        delete autoSearchDialog;
        autoSearchDialog = new AutoSearchDialog(this,
                                                settings.toMap().value("pingCommands").toList().at(0).toString(),
                                                settings.toMap().value("pingResponseOk").toString(),
                                                settings.toMap().value("pingResponseErr").toString());
        autoSearchDialog->setWindowTitle(qApp->applicationName()
                                         .append(". Auto Search \"")
                                         .append(settings.toMap().value("family").toString()
                                                 .append("\"")));
        autoSearchDialog->exec();
    } while(autoSearchDialog->result == 2);

    if(autoSearchDialog->result == 1) {
        QString ip = autoSearchDialog->selectedAddress;
        int port = autoSearchDialog->selectedPort;
        QStringList l = ip.split(QRegularExpression("[.]"), Qt::SkipEmptyParts);
        if (l.size() == 4) {
            setIpAddress(l[0], l[1], l[2], l[3], QString("%1").arg(port));
            deviceAddress = QString("%1.%2.%3.%4:%5").arg(l[0]).arg(l[1]).arg(l[2]).arg(l[3]).arg(port);
            deviceIPAddress = QString("%1.%2.%3.%4").arg(l[0]).arg(l[1]).arg(l[2]).arg(l[3]);
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
    ui->groupBox_Connect->setEnabled(true);
    QString family = settings.toMap().value("family").toString();
    deviceFamily = family;
    setWindowTitle(qApp->applicationName().append(". Connect to \"").append(family).append("\""));
}

void DeviceConnector::reloadDevicesFamily() {
    QStringList saved = RCSettings::settingsList();
    ui->knownDevicesComboBox->clear();
    ui->knownDevicesComboBox->addItems(RCSettings::devicesList());
    saved.sort();
    ui->deviceProtocol->clear();
    ui->deviceProtocol->addItems(saved);
    //ui->deviceProtocol->setItemIcon(0,QIcon(QString(":/images/tray/NetRC-Gray.png")));
    QString family = settings.toMap().value("family").toString();
    if(ui->deviceProtocol->findText(family) > 0) {
        ui->deviceProtocol->setCurrentIndex(ui->deviceProtocol->findText(family));
    }
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
    qDebug() << ui->knownDevicesComboBox->itemText(index);
    QVariant sets = RCSettings::deviceSettings(ui->knownDevicesComboBox->itemText(index));
    qDebug() << ui->knownDevicesComboBox->itemText(index) <<sets;
    if (sets.isValid()) {
        setDevice(sets.toMap().value("deviceFamily", "").toString(),
                sets.toMap().value("deviceName", "").toString(),
                sets.toMap().value("deviceIPAddress", "").toString(),
                sets.toMap().value("devicePort").toUInt());
    }
}


void DeviceConnector::on_pushButton_clicked() {
   RCSettings::removeDevice(QString(ui->knownDevicesComboBox->currentText()));
   ui->knownDevicesComboBox->clear();
   ui->knownDevicesComboBox->addItems(RCSettings::devicesList());
}

