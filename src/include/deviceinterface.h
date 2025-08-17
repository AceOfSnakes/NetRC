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
#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H


#include <QSettings>
#include <QRegularExpression>
#include <QVariant>
#include <QVariantMap>
#include <QtNetwork/QTcpSocket>
using namespace std;

class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    QVariantMap            deviceSettings;
    QString                deviceName;
    QVariantList           pingCommands = {};

    DeviceInterface();
    bool isConnected();
    void reloadSettings();
    bool isPingRs(const QString& data);
    bool isPingOkRs(const QString& data);
    bool isPingErrRs(const QString& data);
    bool isTimeRs(const QString& data);
    bool isDeviceIdRs(const QString& data);
private:
    QString        deviceId;
    QTcpSocket     socket;
    bool           connected;
    bool           crlf;

    QRegularExpression     timestampRegex;
    QRegularExpression     deviceIdRegex;
    QRegularExpression     deviceNameRegex;
    QString                pingResponseOk;
    QString                pingResponseErr;
    QString                pingResponsePlay;
    QString                pingPlayCommand;

    void InterpretString(const QString& data);
    void checkSpecialResponse(const QString&);
private slots:
    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpConnected();
    void tcpDisconnected();
    void readString();
public slots:
    void connectToDevice(const QString &PlayerIpAddress, const unsigned int PlayerIpPort);
    void disconnect();
    bool sendCmd(const QString& cmd);
    void reloadDeviceSettings(QVariantMap settings);
signals:
    void settingsChanged();
    void deviceOffline(bool);
    void commError(QString error);
    void deviceConnected();
    void deviceDisconnected();
    void dataReceived(QString);
    void updateDeviceInfo();
    void updateDisplayInfo(QRegularExpressionMatch);
    void updateDeviceInfo(QRegularExpressionMatch);
    void tx(const QString str);
    void rx(const QString str);
    void specialControl(const QString control, bool enabled);
};

#endif // PLAYERINTERFACE_H
