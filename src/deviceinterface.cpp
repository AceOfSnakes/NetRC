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
#include "crypto.h"
#include <QThread>
#include <QStringList>
#include <QHostInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDebug>

DeviceInterface::DeviceInterface() {
    connected = false;
    // socket
    connect((&socket), SIGNAL(connected()), this, SLOT(tcpConnected()));
    connect((&socket), SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect((&socket), SIGNAL(readyRead()), this, SLOT(readString()));
    connect((&socket), SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
            this,  SLOT(tcpError(QAbstractSocket::SocketError)));
    reloadDeviceSettings(deviceSettings);
}

void DeviceInterface::reloadSettings() {
    pingCommands.append(deviceSettings.value("pingCommands").toList());
    timestampRegex.setPattern(deviceSettings.value("timeRegExp").toString());
    deviceIdRegex.setPattern(deviceSettings.value("deviceIdRegExp").toString());
    deviceNameRegex.setPattern(deviceSettings.value("deviceNameRegExp").toString());
    pingResponseOk = deviceSettings.value("pingResponseOk").toString();
    pingResponseErr = deviceSettings.value("pingResponseErr").toString();
    pingResponsePlay = deviceSettings.value("pingResponsePlay").toString();
    pingPlayCommand = deviceSettings.value("pingPlayCommand").toString();
    // Crypto settings
    crypted = false;
    crypto = nullptr;

    if(deviceSettings.value("crypto").isValid()) {
        crypted = true;
        crypto = new Crypto(cryptoSettings, this);

        connect(crypto, &Crypto::decoded, this, [&](const QString pos) { emit rx(pos, true); } );
        connect(crypto, &Crypto::encoded, this, [&](const QString pos) { emit tx(pos, true); } );
        connect(crypto, &Crypto::info, this, [&](const QString pos) { emit warn(pos, crypted); } );
    }
    // emit signal for device change
    emit warn(deviceSettings.value("family").toString());

    deviceId = QString();
    deviceName = QString();
}

void DeviceInterface::reloadDeviceSettings(QVariantMap  settings) {
    //qDebug() << settings;
    deviceSettings = settings;
    deviceSettings.detach();
    pingCommands.clear();
    reloadSettings();
    emit settingsChanged();
}

void DeviceInterface::updateCryptoSettings(Crypto::CryptoSettings csets) {
    if(crypted) {
        crypto->updateCryptoSettings(csets);
    }
}

void DeviceInterface::connectToDevice(const QString& deviceIpAddress,
                                      const unsigned int deviceIpPort,
                                      Crypto::CryptoSettings csets) {
    if(crypted) {
        crypto->updateCryptoSettings(csets);
    }
    disconnect();
    socket.abort();
    socket.connectToHost(deviceIpAddress, deviceIpPort, QAbstractSocket::ReadWrite,
                         QAbstractSocket::IPv4Protocol);
}

void DeviceInterface::disconnect() {
    connected = false;
    if(socket.state() == QAbstractSocket::ConnectingState ) {
        socket.abort();
    }

    socket.disconnectFromHost();
    socket.close();
}

void DeviceInterface::tcpConnected() {
    connected = true;
    if(!deviceSettings.value("initCmd").isNull()) {
        sendCmd(deviceSettings.value("initCmd").toString());
    }
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

    if(crypted) {
        QByteArray response = QByteArray::fromRawData(data.data(), count);
        emit rxArray(response, true);
        QString res = decrypt(response);
        res= res.trimmed();
        interpretString(res);
    }
    else {
        for(int i = 0; i < count; i++) {
            if (data[i] != '\n' && data[i] != '\r') {
                continue;
            }
            lineLength = i - lineStartPos;
            if (lineLength > 0) {

                receivedString.append((const char*)&data[lineStartPos], 0, lineLength);
                if (receivedString != "") {
                    QString str;
                    str = str.fromUtf8(receivedString.c_str());
                    QRegularExpression re("(\r\n|\r|\n)$");
                    str = str.replace(re, "");

                    emit rx(str);
                    interpretString(str);
                }
                receivedString = "";
            }
            lineStartPos = i + 1;
        }
        if (lineStartPos < count) {
            receivedString.append((const char*)&data[lineStartPos]);
        }
    }
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
    emit err(str);
    emit commError(str);
}

bool DeviceInterface::sendCmd(const QString & cmd) {
    QByteArray newCmd = encrypt(cmd);
    if(crypted) {
        emit txArray(newCmd, true);
    }
    return socket.write(newCmd, newCmd.length()) == newCmd.length();
}

bool DeviceInterface::isDeviceIdRs(const QString& data) {
    return deviceIdRegex.isValid() && !deviceIdRegex.pattern().isEmpty() ?
               deviceIdRegex.match(data).hasMatch() :
               false;
}

bool DeviceInterface::isTimeRs(const QString& data) {
    return timestampRegex.isValid() && !timestampRegex.pattern().isEmpty() ?
               timestampRegex.match(data).hasMatch() :
               false;
}

bool DeviceInterface::isPingErrRs(const QString& data) {
    return pingResponseErr.isEmpty() ? data.isEmpty():
               data.contains(pingResponseErr);
}

bool DeviceInterface::isPingOkRs(const QString& data) {
    return data.contains(pingResponseOk) ;
}

bool DeviceInterface::isPingRs(const QString& data) {
    return isPingErrRs(data) || isPingOkRs(data);
}

void DeviceInterface::interpretString(const QString& dataOrigin) {
    bool deviceNameMatch = deviceNameRegex.isValid() ?
                               deviceNameRegex.match(dataOrigin).hasMatch() : false;

    if(isDeviceIdRs(dataOrigin)) {
        QString newDeviceId = deviceIdRegex.match(dataOrigin).captured(1);
        if(newDeviceId != deviceId) {
            deviceId = deviceIdRegex.match(dataOrigin).captured(1);
            sendCmd(deviceSettings.value("deviceNameRq").toString().replace(QString("%s"),deviceId));
        }
    } else if(deviceNameMatch) {
        deviceName = deviceNameRegex.match(dataOrigin).captured(1);
        emit updateDeviceInfo();
    }

    checkSpecialResponse(dataOrigin);

    if(isPingErrRs(dataOrigin)) {
        emit deviceOffline(true);
        return;
    } else if (isPingOkRs(dataOrigin)) {
        emit deviceOffline(false);
        if(!pingResponsePlay.isEmpty()) {
            if(dataOrigin == pingResponsePlay) {
                sendCmd(pingPlayCommand);
            } else {
                QRegularExpressionMatch rx;
                emit updateDisplayInfo(rx);
            }
        }
        return;
    } else if(isTimeRs(dataOrigin)) {
        emit updateDisplayInfo(timestampRegex.match(dataOrigin));
    }
}

void DeviceInterface::checkSpecialResponse(const QString& response) {
    QVariant special = deviceSettings.value("specialControl");
    if(special.isValid()) {
        QMap<QString,QVariant>map = special.toMap();
        foreach(auto value, map ) {
            QVariant spec = value;
            if(spec.isValid()) {
                if(spec.toMap().value("err").toList().contains(response)) {
                    emit specialControl(map.key(value), false);
                } else if(spec.toMap().value("ok").toList().contains(response)) {
                    emit specialControl(map.key(value), true);
                }
            }
        }
    }
}

QByteArray DeviceInterface::decrypt(QByteArray& data) {
    if(crypted) {
        auto result = crypto->decrypt(data);
        auto res = result.split('\r').at(0).split(('\n')).at(0);
        emit rx(res);
        return res;
    }
    emit rx(data);
    return data;
}

QString DeviceInterface::applyTrailer(QString) {
    return QString().append(deviceSettings.value("crlf", false).toBool() ? "\r\n":
                         deviceSettings.value("lf", false).toBool() ? "\0xD" :"\0xA");
}

QByteArray DeviceInterface::encrypt(const QString& data, const char *) {
    emit tx(data);
    if(crypted) {
        //emit warn(data);
         QByteArray array = crypto->encrypt(
            QString(data).append("\r").toUtf8());
        // Decrypt crypted
        // emit warn("---- Self decrypt start ----");
        // emit rx(crypto->decrypt(array));
        // emit warn("---- Self decrypt end ------");
        // emit tx(array.toHex(' ').toUpper());
        return array;
    }
    return QString(data)
         .append(applyTrailer(data))
         .toLatin1();
}
