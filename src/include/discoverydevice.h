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
#ifndef DISCOVERYDEVICE_H
#define DISCOVERYDEVICE_H

#include <QString>
#include <QtNetwork/QTcpSocket>

class DiscoveryDevice : public QObject
{
    Q_OBJECT
public:
    QString ip;
    unsigned int port;
    DiscoveryDevice();
    virtual void connectDevice(QString ip, unsigned int port);
    virtual int bytesAvailable();
    virtual int write(QString str);
    //virtual int read(char* data, quint64 maxSize);
    virtual QString errorString();
    virtual void close();
    virtual ~DiscoveryDevice();
protected:
    QTcpSocket* socket;
    virtual QString read();
    virtual void newDataToRead() = 0;
protected slots:
    void _newDataToRead();
    //void _DataAvailable(){emit DataAvailable();}
    void _tcpError(QAbstractSocket::SocketError socketError){ emit tcpError(socketError); }
    void _tcpConnected() { emit tcpConnected(); }
    void _tcpDisconnected() { emit tcpDisconnected(); }
signals:
    void dataReceived(const QString&);
    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpConnected();
    void tcpDisconnected();
};
#endif // DISCOVERYDEVICE_H
