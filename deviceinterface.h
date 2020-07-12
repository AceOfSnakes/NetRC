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

    bool IsConnected();
    QVariantList   pingCommands = {"?P"};
    QVariantMap deviceSettings;
private:
    QTcpSocket      socket;
    string          m_ReceivedString;
    bool            connected;
    bool            crlf;
    int             errorCount;
    QRegExp         rxBD;
    void InterpretString(const QString& data);

private slots:

    void TcpError(QAbstractSocket::SocketError socketError);
    void TcpConnected();
    void TcpDisconnected();
    void ReadString();
public slots:
    void ConnectToPlayer(const QString &PlayerIpAddress, const int PlayerIpPort);
    void Disconnect();
    bool SendCmd(const QString& cmd);
    void reloadPlayerSettings(QVariantMap  settings);

signals:
    void SettingsChanged();
    void PlayerOffline(bool);
    void CommError(QString error);
    void Connected();
    void Disconnected();
    void DataReceived(QString);
    void UpdateDisplayInfo(QRegExp&);
};

#endif // PLAYERINTERFACE_H
