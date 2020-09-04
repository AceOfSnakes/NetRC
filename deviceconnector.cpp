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
#include "ui_deviceconnector.h"
#include "rcsettings.h"

DeviceConnector::DeviceConnector(QVariant &sets, QWidget *parent) :
    QDialog(parent),
    settings(sets),
    ui(new Ui::DeviceConnector)
{
    ui->setupUi(this);
    QString family = settings.toMap().value("family").toString();
    setWindowTitle(qApp->applicationName().append(". Connect to ").append(family));
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint) &(~Qt::WindowMaximizeButtonHint));
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
    reloadDevicesFamily();
}

DeviceConnector::~DeviceConnector() {
    delete ui;
}

void DeviceConnector::setDevice(QString deviceFamily, QString device, QString address, int port) {
    ui->deviceProtocol->setCurrentText(deviceFamily);
    ui->line_DeviceName->setText(device);
    QStringList l = address.split(QRegularExpression("[.]"), Qt::SkipEmptyParts);
    if (l.size() == 4) {
        if(port == -1) {
            setIpAddress(l[0], l[1], l[2], l[3], "?");
        } else {
            setIpAddress(l[0], l[1], l[2], l[3], QString().asprintf("%d",port));
        }
    }
}

void DeviceConnector::on_pushButtonAuto_clicked() {
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

void DeviceConnector::on_closeButton_clicked()
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

void DeviceConnector::on_loadConfig_clicked() {
    QString xfile = QFileDialog::getOpenFileName(this, tr("Load RC Settings Protocol"),
                                                 NULL, tr("JSON Files")
                                                 .append(" (*.json);;")
                                                 .append(tr("All files"))
                                                 .append(" (* *.*)"));
    if (!xfile.isEmpty()) {
        QFile file(xfile);
        QVariant var = RCSettings::load(file);
        qDebug()<<var;
        select(var);

        reloadDevicesFamily();
        on_deviceProtocol_currentIndexChanged(var.toMap().value("family").toString());
    }
}

void DeviceConnector::select(QVariant vars) {
    settings.swap(vars);
    ui->groupBox_Connect->setEnabled(true);
    QString family = settings.toMap().value("family").toString();
    deviceFamily = family;
    setWindowTitle(qApp->applicationName().append(". Connect to ").append(family));
}

void DeviceConnector::reloadDevicesFamily() {
    QStringList saved = RCSettings::settingsList();
    saved.sort();
    ui->deviceProtocol->clear();
    ui->deviceProtocol->addItems(saved);
    QString family = settings.toMap().value("family").toString();
    if(ui->deviceProtocol->findText(family) > 0) {
        ui->deviceProtocol->setCurrentIndex(ui->deviceProtocol->findText(family));
    }
}

void DeviceConnector::on_deviceProtocol_currentIndexChanged(const QString &arg1) {
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

void DeviceConnector::on_deleteFamily_clicked() {
    if(!ui->deviceProtocol->currentText().isEmpty()) {
        QMessageBox::StandardButton button = QMessageBox::information(
                    this,
                    QString("Remove \"").append(ui->deviceProtocol->currentText()).append("\""),
                    QString("Remove \"").append(ui->deviceProtocol->currentText()).append("\" from available device protocols?    ")
                    .append("\nAll permanently saved assosiated devices will also be deleted    "),
                    QMessageBox::Yes | QMessageBox::No ,
                    QMessageBox::Yes);
        if(button == QMessageBox::Yes) {
            RCSettings::remove(ui->deviceProtocol->currentText());
            reloadDevicesFamily();
        }
    }
}

void DeviceConnector::on_deleteFamily_pressed() {
    on_deleteFamily_clicked();
}
