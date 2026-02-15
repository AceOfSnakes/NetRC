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
#include <QLineEdit>
#include <QRegularExpression>
#include <QGridLayout>
#include <QVariant>
#include "crypto.h"
#include <QRegularExpression>

namespace Ui {
class DeviceConnector;
}

class DeviceConnector : public QDialog
{
    Q_OBJECT
private:
    QRegularExpression reIP; //("[.]");
    void applyCryptoToUI(QGridLayout * layout);
    void applyCryptoBlockToUI(QGridLayout * layout, QString);
public:
    explicit DeviceConnector(QVariant & sets, QWidget *parent = nullptr);
    ~DeviceConnector();
    Crypto::CryptoSettings cryptoSettings;
    QString  device;
    QString  deviceAddress;
    QString  deviceIPAddress;
    boolean  cryptoEnabled = false;
    unsigned int devicePort;
    QString  deviceFamily;
    QPixmap  img;
    QVariant settings;
    void setDevice(QString deviceFamily, QString device, QString address,
                   unsigned int port, QPixmap logo, Crypto::CryptoSettings cryptosettings);

    void loadLogo();
    
    void reloadCryptoSettings(bool resetCrypto = true);
    
private slots:
    void resetCryptoToDefault();
    void autoSearchClicked();
    void applyButtonClicked();
    void loadConfigClicked();
    void deviceProtocolCurrentIndexChanged(const QString &);
    void deleteFamilyClicked();

    void onKnownDevicesComboBoxCurrentIndexChanged(int index);

    void onRemoveKnownDevice();
    void onLogoUploaded(bool);
    void onLogoRemoved(bool);
private:
    Ui::DeviceConnector *ui;
    void reloadDevicesFamily();
    QString getIpAddress();
    QLineEdit * createCryptoWidget(QByteArray value, bool editable = false);
    QLineEdit * createCryptoWidget(QString & value, bool editable = false);
    QWidget* createCryptoWidget(int & value, bool editable = false);
    void applyCryptoWidgetStyle(QWidget * widget, bool editable = false);
    void select(const QVariant action);
    void setIpAddress(QString ip1, QString ip2, QString ip3, QString ip4, QString port);
};

#endif // DEVICECONNECTOR_H
