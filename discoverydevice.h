#ifndef DISCOVERYDEVICE_H
#define DISCOVERYDEVICE_H

#include <QString>
#include <QtNetwork/QTcpSocket>

class DiscoveryDevice : public QObject
{
    Q_OBJECT
public:
    QString ip;
    int port;
    DiscoveryDevice();
    virtual void Connect(QString ip, int port);
    virtual int bytesAvailable();
    virtual int write(QString str);
    //virtual int read(char* data, quint64 maxSize);
    virtual QString errorString();
    virtual void close();
    virtual ~DiscoveryDevice();
protected:
    QTcpSocket* socket;
    virtual QString read();
    virtual void NewDataToRead() = 0;
protected slots:
    void _NewDataToRead(){NewDataToRead();}
    //void _DataAvailable(){emit DataAvailable();}
    void _TcpError(QAbstractSocket::SocketError socketError){emit TcpError(socketError);}
    void _TcpConnected(){emit TcpConnected();}
    void _TcpDisconnected(){TcpDisconnected();}
signals:
    void DataReceived(const QString&);
    void TcpError(QAbstractSocket::SocketError socketError);
    void TcpConnected();
    void TcpDisconnected();
};
#endif // DISCOVERYDEVICE_H
