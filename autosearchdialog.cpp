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
#include "autosearchdialog.h"
#include <string>
#include "ui_autosearchdialog.h"
#include <QHostAddress>
#include <QThread>
#include <QClipboard>
#include <QRegExp>
#include <QtNetwork>
#include <QtXml>
#include <QMenu>

string trim(const string &t, const string &ws) {
    string str = t;
    size_t found;
    found = str.find_last_not_of(ws);
    if (found != string::npos) {
        str.erase(found+1);
    }
    else {
        return "";
    }
    return str.erase(0, str.find_first_not_of(ws.c_str()));
}

RemoteDevice::RemoteDevice() {
    port = 0;
    socket = NULL;
}

RemoteDevice::RemoteDevice(QString &url):RemoteDevice()
{
    m_url = url;
}

void RemoteDevice::connectRemoteDevice(QString ip, int port) {
    this->port = port;
    this->ip = ip;
    delete socket;
    socket = new QTcpSocket();
    connect((socket), SIGNAL(connected()), this, SLOT(_tcpConnected()));
    connect((socket), SIGNAL(disconnected()), this, SLOT(_tcpDisconnected()));
    connect((socket), SIGNAL(readyRead()), this, SLOT(_dataAvailable()));
    connect((socket), SIGNAL(error(QAbstractSocket::SocketError)), this,  SLOT(_tcpError(QAbstractSocket::SocketError)));
    socket->connectToHost(ip, port);
}

RemoteDevice::~RemoteDevice() {
    delete socket;
    socket = NULL;
}

AutoSearchDialog::AutoSearchDialog(QWidget *parent, QString pingCommand, QString pingResponseStart, QString pingResponseStartOff) :
    QDialog(parent),
    result(0),
    selectedPort(0),
    pingCommand(pingCommand),
    pingResponseStart(pingResponseStart),
    pingResponseStartOff(pingResponseStartOff),
    groupAddress("239.255.255.250"),
    ui(new Ui::AutoSearchDialog)
{
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
    ui->setupUi(this);

    ui->label->setVisible(false);
    ui->timeLabel->setVisible(false);

    foreach (const QNetworkInterface& iface, QNetworkInterface::allInterfaces()) {
        if (iface.flags() & QNetworkInterface::IsUp && !(iface.flags() & QNetworkInterface::IsLoopBack)) {
            QUdpSocket* socket = new QUdpSocket(this);
            if (!socket->bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress)) {
                delete socket;
                continue;
            }
            if (!socket->joinMulticastGroup(groupAddress)) {
                delete socket;
                continue;
            }
            socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 5);
            socket->setMulticastInterface(iface);

            connect(socket, SIGNAL(readyRead()),
                    this, SLOT(processPendingDatagrams()));
            multicatsSockets.push_back(socket);
        }
    }
    sendMsg();
}

AutoSearchDialog::~AutoSearchDialog() {
    foreach (RemoteDevice* tmp, remoteDevices) {
        delete tmp;
    }
    remoteDevices.clear();
    foreach (RemoteDevice* tmp, deviceInList) {
        delete tmp;
    }
    deviceInList.clear();
    delete ui;
}

void AutoSearchDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void AutoSearchDialog::newDevice(QString , QString ip, QString location) {
    QUrl url = QUrl(location);
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // the HTTP request
    QNetworkRequest req(QUrl(QString(location.toLatin1())));
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    QString manufacturer;
    QString friendlyName;
    QString modelName;
    QString remoteSupported("0");
    QString remotePort(QString::number(url.port()));

    if (reply->error() == QNetworkReply::NoError) {
        //success
        //Get your xml into xmlText(you can use QString instead og QByteArray)
        QString data = reply->readAll();
        QDomDocument document;
        document.setContent(data);
        QDomNodeList nodes = document.childNodes();
        for (int i = 0; i < nodes.count(); i++) {
            if (nodes.at(i).nodeName() == "root") {
                QDomNodeList nodes1 = nodes.at(i).childNodes();
                for (int j = 0; j < nodes1.count(); j++) {
                    if (nodes1.at(j).nodeName() == "device") {
                        QDomNodeList nodes2 = nodes1.at(j).childNodes();
                        for (int k = 0; k < nodes2.count(); k++) {
                            QString name = nodes2.at(k).nodeName();
                            QDomNodeList nodes3 = nodes2.at(k).childNodes();
                            QString text;
                            for (int m = 0; m < nodes3.count(); m++) {
                                if (nodes3.at(m).isText())
                                    text += nodes3.at(m).nodeValue();
                            }
                            if (name == "manufacturer" || name == "manufacture") {
                                manufacturer = text;
                            }
                            else if (name == "friendlyName") {
                                friendlyName = text;
                            }
                            else if (name == "modelName")  {
                                modelName = text;
                            } else if(name.endsWith("X_ipRemoteReady")) {
                                remoteSupported = text;
                            } else if(name.endsWith("X_ipRemoteTcpPort")) {
                                remotePort = text;
                            }
                        }
                    }
                }
            }
        }
        delete reply;
    }
    else {
        delete reply;
        eventLoop.quit();
        return;
    }
    eventLoop.quit();
    QString deviceKey = modelName.append("/").append(QString::number(url.port()));
    RemoteDevice* device = new RemoteDevice(location);
    connect((device), SIGNAL(tcpConnected()), this, SLOT(tcpConnected()));
    connect((device), SIGNAL(tcpDisconnected()), this, SLOT(tcpDisconnected()));
    connect((device), SIGNAL(dataAvailable()), this, SLOT(readString()));
    connect((device), SIGNAL(tcpError(QAbstractSocket::SocketError)), this,  SLOT(tcpError(QAbstractSocket::SocketError)));
    device->connectRemoteDevice(ip, url.port());
    remoteDevices.insert(deviceKey,device);
}

void AutoSearchDialog::tcpConnected()
{
    QObject* sender = QObject::sender();
    RemoteDevice* device = dynamic_cast<RemoteDevice*>(sender);
    device->socket->write(QString().append(pingCommand).append("\r\n").toLatin1().data());
}

void AutoSearchDialog::tcpDisconnected()
{
}

QString AutoSearchDialog::removeDevice(QMap<QString,RemoteDevice*>  &m_RemoteDevices,RemoteDevice* device) {
    foreach(QString key,m_RemoteDevices.keys()) {
        if(device == m_RemoteDevices.value(key)) {
            m_RemoteDevices.remove(key);
            int index=key.lastIndexOf("/");
            key.truncate(index);
            return key;
        }
    }
    return QString();
}
void AutoSearchDialog::reconnect(QString & key,QString & ip,int port,RemoteDevice* device){
    if (key != NULL && (port == 23 || port != 8102))
    {
        device = new RemoteDevice();
        connect((device), SIGNAL(tcpConnected()), this, SLOT(tcpConnected()));
        connect((device), SIGNAL(tcpDisconnected()), this, SLOT(tcpDisconnected()));
        connect((device), SIGNAL(dataAvailable()), this, SLOT(readString()));
        connect((device), SIGNAL(tcpError(QAbstractSocket::SocketError)), this,  SLOT(tcpError(QAbstractSocket::SocketError)));
        if(port == 23) {
            device->connectRemoteDevice(ip, 8102);
            remoteDevices.insert(key.append("/8102"), device);
        } else {
            device->connectRemoteDevice(ip, 23);
            remoteDevices.insert(key.append("/23"), device);
        }
    }
}
void AutoSearchDialog::readString() {
    QObject* sender = QObject::sender();
    RemoteDevice* device = dynamic_cast<RemoteDevice*>(sender);
    int count = device->socket->bytesAvailable();
    std::vector<char> data;
    data.resize(count + 1);
    device->socket->read(&data[0], count);
    data[count] = '\0';
    string m_ReceivedString="";
    m_ReceivedString.append((const char*)&data[0], 0, count);
    m_ReceivedString = trim(m_ReceivedString, "\r");
    m_ReceivedString = trim(m_ReceivedString, "\n");
    QString str;
    str = str.fromUtf8(m_ReceivedString.c_str());
    if (str.contains(pingResponseStart)||
            (!pingResponseStartOff.isEmpty() && str.contains(pingResponseStartOff))) {
        foreach(RemoteDevice *dev ,deviceInList) {
            if(QString::compare(device->ip,dev->ip)==0 && device->port == dev->port) {
                qDebug()<<"already in list"<<device->ip<<device->port;
                device->socket->close();
                device->socket->disconnect();
                device->deleteLater();
                deviceInList.removeOne(device);
                return;
            }
        }

        RemoteDevice* rd = new RemoteDevice();
        rd->ip = device->ip;
        rd->port = device->port;
        QString key = removeDevice(remoteDevices, device);
        deviceInList.append(rd);
        ui->listWidget->addItem(QString("%1 (%2:%3)").arg(key).arg(device->ip).arg(device->port));
        if (ui->listWidget->count() == 1) {
            ui->listWidget->setCurrentRow(0);
            selectedAddress = device->ip;
            selectedPort = device->port;
        }
        ui->listWidget->item(ui->listWidget->count() - 1)->setData(Qt::UserRole, device->ip);
        ui->listWidget->item(ui->listWidget->count() - 1)->setData(Qt::UserRole + 1, device->port);
        ui->listWidget->item(ui->listWidget->count() - 1)->setData(Qt::UserRole + 2, key);

        device->socket->close();
        device->socket->disconnect();
        device->deleteLater();
        deviceInList.removeOne(device);
        return;
    }

    int port = device->port;
    QString ip = device->ip;
    QString key = removeDevice(remoteDevices,device);
    reconnect(key,ip,port,device);
}

void AutoSearchDialog::tcpError(QAbstractSocket::SocketError socketError) {
    QObject* sender = QObject::sender();
    RemoteDevice* device = dynamic_cast<RemoteDevice*>(sender);
    QString str;
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        str = QString("Host closed connection: %1").arg(device->socket->errorString());
        break;
    case QAbstractSocket::HostNotFoundError:
        str = QString("Host not found: %1").arg(device->socket->errorString());
        break;
    case QAbstractSocket::ConnectionRefusedError:
        str = QString("Host refused connection: %1").arg(device->socket->errorString());
        break;
    default:
        str = QString("The following error occurred: %1.").arg(device->socket->errorString());
    }
    int port = device->port;
    QString ip = device->ip;
    QString key = removeDevice(remoteDevices,device);
    device->deleteLater();
    reconnect(key,ip,port,device);
}
void AutoSearchDialog::on_CancelButton_clicked()
{
    result = 0;
    close();
}

void AutoSearchDialog::on_continueButton_clicked()
{
    result = 1;
    loadRowData(ui->listWidget->currentRow());;
    close();
}

void AutoSearchDialog::on_repeatButton_clicked()
{
    result = 2;
    close();
}
void AutoSearchDialog::loadRowData(int row) {
    if(row >= 0 ) {
        selectedAddress = ui->listWidget->item(row)->data(Qt::UserRole).toString();
        selectedPort = ui->listWidget->item(row)->data(Qt::UserRole + 1).toInt();
        selectedDevice = ui->listWidget->item(row)->data(Qt::UserRole + 2).toString();
    }
}

void AutoSearchDialog::on_listWidget_clicked(const QModelIndex &index) {
    loadRowData(index.row());
}

void AutoSearchDialog::on_listWidget_doubleClicked(const QModelIndex &index) {
    result = 1;
    on_listWidget_clicked(index);
    close();
}

void AutoSearchDialog::closeEvent(QCloseEvent *event) {
    foreach(QUdpSocket* socket, multicatsSockets) {
        socket->leaveMulticastGroup(groupAddress);
        socket->close();
        delete socket;
    }
    multicatsSockets.clear();
    QWidget::closeEvent(event);
}

void AutoSearchDialog::processPendingDatagrams() {
    foreach( QUdpSocket* socket, multicatsSockets) {
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            QHostAddress remoteAddr;
            datagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(datagram.data(), datagram.size(), &remoteAddr);
            QString data = QString(datagram);
            if (data.contains("200 OK", Qt::CaseInsensitive) && data.contains("rootdevice", Qt::CaseInsensitive)) {
                QStringList ll = data.split(QRegExp("[\n\r]"), Qt::SkipEmptyParts);
                QString location;
                foreach (QString s, ll) {
                    if (s.startsWith("LOCATION: ", Qt::CaseInsensitive)) {
                        location = s.mid(10);
                        break;
                    }
                }
                newDevice("", remoteAddr.toString(), location);
            }
        }
    }
}

void AutoSearchDialog::sendMsg() {
    QString datagram;
    datagram.append("M-SEARCH * HTTP/1.1\r\nHOST: ")
            .append(groupAddress.toString())
            .append(":1900\r\n"
                    "MAN: \"ssdp:discover\"\r\n"
                    "MX: 1\r\n"
                    "ST: upnp:rootdevice\r\n"
                    "USER-AGENT: ")
            .append(qAppName())
            .append("\r\n\r\n\r\n");

    foreach(QUdpSocket* socket, multicatsSockets) {
        socket->writeDatagram(datagram.toUtf8(), groupAddress, 1900);
    }
}

