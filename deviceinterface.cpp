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
#include "deviceinterface.h"
#include <QTextCodec>
#include <QThread>
#include <QStringList>
#include <QJsonDocument>
#include <QRegExp>
#include <QDebug>
string trim(const string &t, const string &ws);
DeviceInterface::DeviceInterface():errorCount(0) {
    rxBD.setPattern(deviceSettings.value("timeRegExp").toString());
    connected = false;
    // socket
    connect((&socket), SIGNAL(connected()), this, SLOT(tcpConnected()));
    connect((&socket), SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect((&socket), SIGNAL(readyRead()), this, SLOT(readString()));
    connect((&socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(tcpError(QAbstractSocket::SocketError)));
    reloadDeviceSettings(deviceSettings);
}

void DeviceInterface::reloadDeviceSettings(QVariantMap  settings) {
    deviceSettings.clear();
    deviceSettings.unite(settings);
    pingCommands.clear();
    pingCommands.append(deviceSettings.value("pingCommands").toList());
    rxBD.setPattern(deviceSettings.value("timeRegExp").toString());
    emit settingsChanged();
}

void DeviceInterface::connectToDevice(const QString& PlayerIpAddress, const int PlayerIpPort) {
    socket.disconnectFromHost();
    socket.connectToHost(PlayerIpAddress, PlayerIpPort);
}

void DeviceInterface::disconnect() {
    connected = false;
    socket.disconnectFromHost();
    socket.close();
}

void DeviceInterface::tcpConnected() {
    connected = true;
    emit deviceConnected();
}

void DeviceInterface::tcpDisconnected() {
    connected = false;
    emit deviceDisconnected();
}

bool DeviceInterface::isConnected() {
    return connected;
}

void DeviceInterface::readString() {
    // read all available data
    int count = socket.bytesAvailable();
    std::vector<char> data;
    data.resize(count + 1);
    socket.read(&data[0], count);
    data[count] = '\0';

    // split lines
    int lineLength = 0;
    string receivedString;
    int lineStartPos = 0;
    for(int i = 0; i < count; i++)
    {
        if (data[i] != '\n' && data[i] != '\r')
        {
            continue;
        }
        lineLength = i - lineStartPos;
        if (lineLength > 0)
        {
            receivedString.append((const char*)&data[lineStartPos], 0, lineLength);
            receivedString = trim(receivedString, "\r");
            receivedString = trim(receivedString, "\n");
            if (receivedString != "")             {
                QString str;
                str = str.fromUtf8(receivedString.c_str());
                qDebug() << QString("What %1 count %2").arg(str).arg(count);
                InterpretString(str);
                emit dataReceived(str);
            }
            receivedString = "";
        }
        lineStartPos = i + 1;
    }
    if (lineStartPos < count)
        receivedString.append((const char*)&data[lineStartPos]);
}

void DeviceInterface::tcpError(QAbstractSocket::SocketError socketError) {
    QString str;
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        str = QString("Host closed connection: %1").arg(socket.errorString());
        break;
    case QAbstractSocket::HostNotFoundError:
        str = QString("Host not found: %1").arg(socket.errorString());
        break;
    case QAbstractSocket::ConnectionRefusedError:
        str = QString("Host refused connection: %1").arg(socket.errorString());
        break;
    default:
        str = QString("The following error occurred: %1.").arg(socket.errorString());
    }
    emit commError(str);
}

bool DeviceInterface::sendCmd(const QString& cmd) {
    QString tmp = cmd + (deviceSettings.value("crlf",true).toBool()?"\r\n":"\r");
    qDebug()<<tmp;
    return socket.write(tmp.toLatin1(), tmp.length()) == tmp.length();
}

void DeviceInterface::InterpretString(const QString& data) {
    bool timeMatch = rxBD.exactMatch(data);
    //qDebug()<< "interpret" << data << deviceSettings.value("pingResponseErr").toString()<<"error count"<<errorCount ;
    if(data.contains(deviceSettings.value("pingResponseErr").toString())) {
        //qDebug()<< "error";
        errorCount ++;
        if(errorCount == pingCommands.size()) {
            qDebug()<< "offline !!!";
            emit deviceOffline(true);
        } else if(errorCount > pingCommands.size()) {
            errorCount = pingCommands.size();
        }
        return;
    } else if (data.contains(deviceSettings.value("pingResponseOk").toString())) {
        //qDebug()<< "ok";
        if(!deviceSettings.value("initCmd").isNull()) {
            sendCmd(deviceSettings.value("initCmd").toString());
        }
        emit deviceOffline(false);
        return;
    } else if(timeMatch) {
        //qDebug()<< "time";{
        emit updateDisplayInfo(rxBD);
    }
    errorCount = 0;
}

