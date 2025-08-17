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
#ifndef DEVICECONNECTOR_H
#define DEVICECONNECTOR_H

#include <QDialog>
#include <QVariant>
#include <QRegularExpression>

namespace Ui {
class DeviceConnector;
}

class DeviceConnector : public QDialog
{
    Q_OBJECT
private:
public:
    explicit DeviceConnector(QVariant & sets, QWidget *parent = nullptr);
    ~DeviceConnector();
    QString  device;
    QString  deviceAddress;
    QString  deviceIPAddress;
    unsigned int devicePort;
    QString  deviceFamily;
    QPixmap  img;
    QVariant settings;
    void setDevice(QString deviceFamily, QString device, QString address,
                   unsigned int port, QPixmap logo);
    void loadLogo();

private slots:
    void autoSearchClicked();
    void applyButtonClicked();
    void loadConfigClicked();
    void deviceProtocolCurrentIndexChanged(const QString &);
    void deleteFamilyClicked();

    void onKnownDevicesComboBoxCurrentIndexChanged(int index);

    void on_pushButton_clicked();
    void onLogoUploaded(bool);
    void onLogoRemoved(bool);
private:
    Ui::DeviceConnector *ui;
    void reloadDevicesFamily();
    QString getIpAddress();
    void select(const QVariant action);
    void setIpAddress(QString ip1, QString ip2, QString ip3, QString ip4, QString port);
};

#endif // DEVICECONNECTOR_H
