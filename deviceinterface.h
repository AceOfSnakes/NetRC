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
#include <QRegExp>
#include <QVariant>
#include <QVariantMap>
#include <QtNetwork/QTcpSocket>
#include <string>

using namespace std;

class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    DeviceInterface();

    bool isConnected();
    QVariantList   pingCommands = {};
    QVariantMap    deviceSettings;
private:
    QTcpSocket     socket;
    //string         receivedString;
    bool           connected;
    bool           crlf;
    int            errorCount;
    QRegExp        rxBD;
    void InterpretString(const QString& data);

private slots:

    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpConnected();
    void tcpDisconnected();
    void readString();
public slots:
    void connectToDevice(const QString &PlayerIpAddress, const int PlayerIpPort);
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
    void updateDisplayInfo(QRegExp&);
};

#endif // PLAYERINTERFACE_H
