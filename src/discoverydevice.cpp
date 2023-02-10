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
#include "discoverydevice.h"

DiscoveryDevice::DiscoveryDevice() {
    port = 0;
    socket = NULL;
}

void DiscoveryDevice::connectDevice(QString ip, unsigned int port) {
    this->port = port;
    this->ip = ip;
    delete socket;
    socket = new QTcpSocket();
    connect((socket), SIGNAL(connected()), this, SLOT(_tcpConnected()));
    connect((socket), SIGNAL(disconnected()), this, SLOT(_tcpDisconnected()));
    connect((socket), SIGNAL(readyRead()), this, SLOT(_newDataToRead()));
    connect((socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(_tcpError(QAbstractSocket::SocketError)));
    socket->connectToHost(ip, port);
}

int DiscoveryDevice::bytesAvailable() {
    return socket->bytesAvailable();
}

int DiscoveryDevice::write(QString str) {
    return socket->write(str.toLatin1());
}


QString DiscoveryDevice::read() {
    QByteArray data = socket->readAll();
    return QString::fromUtf8(data);
}

void DiscoveryDevice::_newDataToRead(){newDataToRead();}

QString DiscoveryDevice::errorString() {
    return socket->errorString();
}

void DiscoveryDevice::close() {
    socket->close();
    socket->disconnect();
}

DiscoveryDevice::~DiscoveryDevice() {
    delete socket;
    socket = NULL;
}
