#include "discoverydevice.h"

DiscoveryDevice::DiscoveryDevice() {
    port = 0;
    socket = NULL;
}

void DiscoveryDevice::Connect(QString ip, int port)
{
    this->port = port;
    this->ip = ip;
    delete socket;
    socket = new QTcpSocket();
    connect((socket), SIGNAL(connected()), this, SLOT(_TcpConnected()));
    connect((socket), SIGNAL(disconnected()), this, SLOT(_TcpDisconnected()));
    connect((socket), SIGNAL(readyRead()), this, SLOT(_NewDataToRead()));
    connect((socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(_TcpError(QAbstractSocket::SocketError)));
    socket->connectToHost(ip, port);
}

int DiscoveryDevice::bytesAvailable()
{
    return socket->bytesAvailable();
}

int DiscoveryDevice::write(QString str)
{
    return socket->write(str.toLatin1());
}


QString DiscoveryDevice::read()
{
    QByteArray data = socket->readAll();
    return QString::fromUtf8(data);
}

QString DiscoveryDevice::errorString()
{
    return socket->errorString();
}

void DiscoveryDevice::close()
{
    socket->close();
    socket->disconnect();
}

DiscoveryDevice::~DiscoveryDevice()
{
    delete socket;
    socket = NULL;
}
