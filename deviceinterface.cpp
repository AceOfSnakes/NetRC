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

DeviceInterface::DeviceInterface():errorCount(0) {
    rxBD.setPattern(deviceSettings.value("timeRegExp").toString());
    connected = false;
    // socket
    connect((&socket), SIGNAL(connected()), this, SLOT(TcpConnected()));
    connect((&socket), SIGNAL(disconnected()), this, SLOT(TcpDisconnected()));
    connect((&socket), SIGNAL(readyRead()), this, SLOT(ReadString()));
    connect((&socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(TcpError(QAbstractSocket::SocketError)));
    reloadPlayerSettings(deviceSettings);
}

void DeviceInterface::reloadPlayerSettings(QVariantMap  settings) {
    deviceSettings.clear();
    deviceSettings.unite(settings);
    pingCommands.clear();
    pingCommands.append(deviceSettings.value("pingCommands").toList());
    rxBD.setPattern(deviceSettings.value("timeRegExp").toString());
    emit SettingsChanged();
}

void DeviceInterface::ConnectToPlayer(const QString& PlayerIpAddress, const int PlayerIpPort) {
    socket.connectToHost(PlayerIpAddress, PlayerIpPort);
}

void DeviceInterface::Disconnect() {
    connected = false;
    socket.disconnectFromHost();
    socket.close();
}

void DeviceInterface::TcpConnected() {
    connected = true;
    emit Connected();
}

void DeviceInterface::TcpDisconnected() {
    connected = false;
    emit Disconnected();
}

bool DeviceInterface::IsConnected() {
    return connected;
}

void DeviceInterface::ReadString() {
    // read all available data
    int count = socket.bytesAvailable();
    std::vector<char> data;
    data.resize(count + 1);
    socket.read(&data[0], count);
    data[count] = '\0';

    // split lines
    int lineLength = 0;
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
            m_ReceivedString.append((const char*)&data[lineStartPos], 0, lineLength);
            //m_ReceivedString = trim(m_ReceivedString, "\r");
            //m_ReceivedString = trim(m_ReceivedString, "\n");
            if (m_ReceivedString != "")
            {
                QString str;
                str = str.fromUtf8(m_ReceivedString.c_str());
                InterpretString(str);
                emit DataReceived(str);
            }
            m_ReceivedString = "";
        }
        lineStartPos = i + 1;
    }
    if (lineStartPos < count)
        m_ReceivedString.append((const char*)&data[lineStartPos]);
}

void DeviceInterface::TcpError(QAbstractSocket::SocketError socketError) {
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
    emit CommError(str);
}

bool DeviceInterface::SendCmd(const QString& cmd) {
    QString tmp = cmd + (deviceSettings.value("crlf",true).toBool()?"\r\n":"\r");
    qDebug()<<tmp;
    return socket.write(tmp.toLatin1(), tmp.length()) == tmp.length();
}

void DeviceInterface::InterpretString(const QString& data) {
    bool timeMatch = rxBD.exactMatch(data);
    if (data.contains(deviceSettings.value("pingResponseOk").toString())) {
        if(!deviceSettings.value("initCmd").isNull()) {
            SendCmd(deviceSettings.value("initCmd").toString());
        }
        emit PlayerOffline(false);
    } else if(data.contains(deviceSettings.value("pingResponseErr").toString())) {
        errorCount ++;
        if(errorCount == pingCommands.size()) {
            emit PlayerOffline(true);
        }
        else if(errorCount > pingCommands.size()) {
            errorCount = pingCommands.size();
        }
        return;
    } else if(timeMatch) {
        emit UpdateDisplayInfo(rxBD);
    }
    errorCount = 0;
}

