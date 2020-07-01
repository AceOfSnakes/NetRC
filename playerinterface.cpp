#include "playerinterface.h"
#include <QTextCodec>
#include <QThread>
#include <QStringList>
#include <QJsonDocument>
#include <QRegExp>



//string trim(const string &t, const string &ws);

PlayerInterface::PlayerInterface():m_error_count(0) {
    rxBD.setPattern(m_PlayerSettings.value("timeRegExp").toString());
    m_Connected = false;
    // socket
    connect((&m_Socket), SIGNAL(connected()), this, SLOT(TcpConnected()));
    connect((&m_Socket), SIGNAL(disconnected()), this, SLOT(TcpDisconnected()));
    connect((&m_Socket), SIGNAL(readyRead()), this, SLOT(ReadString()));
    connect((&m_Socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(TcpError(QAbstractSocket::SocketError)));
    reloadPlayerSettings(m_PlayerSettings);
}

void PlayerInterface::reloadPlayerSettings(QVariantMap  settings) {
    m_PlayerSettings.clear();
    m_PlayerSettings.unite(settings);
    m_ping_commands.clear();
    m_ping_commands.append(m_PlayerSettings.value("pingCommands").toList());
    rxBD.setPattern(m_PlayerSettings.value("timeRegExp").toString());
    emit SettingsChanged();
}

void PlayerInterface::ConnectToPlayer(const QString& PlayerIpAddress, const int PlayerIpPort) {
    m_Socket.connectToHost(PlayerIpAddress, PlayerIpPort);
}

void PlayerInterface::Disconnect() {
    m_Connected = false;
    m_Socket.disconnectFromHost();
    m_Socket.close();
}

void PlayerInterface::TcpConnected() {
    m_Connected = true;
    emit Connected();
}

void PlayerInterface::TcpDisconnected() {
    m_Connected = false;
    emit Disconnected();
}

bool PlayerInterface::IsConnected() {
    return m_Connected;
}

void PlayerInterface::ReadString() {
    // read all available data
    int count = m_Socket.bytesAvailable();
    std::vector<char> data;
    data.resize(count + 1);
    m_Socket.read(&data[0], count);
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

void PlayerInterface::TcpError(QAbstractSocket::SocketError socketError) {
    QString str;
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        str = QString("Host closed connection: %1").arg(m_Socket.errorString());
        break;
    case QAbstractSocket::HostNotFoundError:
        str = QString("Host not found: %1").arg(m_Socket.errorString());
        break;
    case QAbstractSocket::ConnectionRefusedError:
        str = QString("Host refused connection: %1").arg(m_Socket.errorString());
        break;
    default:
        str = QString("The following error occurred: %1.").arg(m_Socket.errorString());
    }
    emit CommError(str);
}

bool PlayerInterface::SendCmd(const QString& cmd) {
    CmdToBeSend(cmd);
    QString tmp = cmd + (m_PlayerSettings.value("crlf",true).toBool()?"\r\n":"\r");
    qDebug()<<tmp;
    return m_Socket.write(tmp.toLatin1(), tmp.length()) == tmp.length();

}

void PlayerInterface::InterpretString(const QString& data) {
    bool timeMatch = rxBD.exactMatch(data);
    if (data.contains(m_PlayerSettings.value("pingResponseOk").toString())) {
        if(!m_PlayerSettings.value("initCmd").isNull()) {
            SendCmd(m_PlayerSettings.value("initCmd").toString());
        }
        emit PlayerOffline(false);
    } else if(data.contains(m_PlayerSettings.value("pingResponseErr").toString())) {
        m_error_count ++;
        if(m_error_count == m_ping_commands.size()) {
            emit PlayerOffline(true);
        }
        else if(m_error_count > m_ping_commands.size()) {
            m_error_count = m_ping_commands.size();
        }
        return;
    } else if(timeMatch) {
        emit UpdateDisplayInfo(rxBD);
    }
    m_error_count = 0;
}

