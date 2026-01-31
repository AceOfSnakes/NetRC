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
#ifndef AUTOSEARCHDIALOG_H
#define AUTOSEARCHDIALOG_H

#include <QDialog>
#include <QNetworkReply>
#include <QDebug>
#include <QModelIndex>
#include <QLibrary>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QSslSocket>
#include <QUdpSocket>
#include <QListWidgetItem>
#include <QSettings>

using namespace std;
namespace Ui {
class AutoSearchDialog;
}

class RemoteDevice : public QObject
{
    Q_OBJECT
    QString m_url;
public:
    RemoteDevice();
    RemoteDevice(QString &url);
    void connectRemoteDevice(QString ip, unsigned int port);
    ~RemoteDevice();
    QString ip;
    unsigned int port;
    QTcpSocket* socket = nullptr;
private slots:
    void _dataAvailable(){emit dataAvailable();}
    void _tcpError(QAbstractSocket::SocketError socketError){emit tcpError(socketError);}
    void _tcpConnected(){emit tcpConnected();}
    void _tcpDisconnected(){emit tcpDisconnected();}
signals:
    void dataAvailable();
    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpConnected();
    void tcpDisconnected();
};

class AutoSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutoSearchDialog(QWidget *parent = 0, QString pingCommand = "?RGD",
                              QString pingResponseStart = "RGD", QString pingResponseStartOff = "",
                              int prefferedPort = 0);
    ~AutoSearchDialog();

    int                     result;
    QString                 selectedFamily;
    QString                 selectedAddress;
    QString                 selectedDevice;
    int                     selectedPort;
    int                     prefferedPort;
    QVector<RemoteDevice*>  deviceInList;

protected:
    void changeEvent(QEvent *e);
    QString pingCommand;
    QString pingResponseStart;
    QString pingResponseStartOff;

    QMap<QString,RemoteDevice*>  remoteDevices;
    QVector<QUdpSocket*>    multicatsSockets;
    QHostAddress            groupAddress;
    QString removeDevice(QMap<QString,RemoteDevice*> &remoteDevices, RemoteDevice* device);
    void sendMsg();
    void reconnect(QString & key,QString & ip,int port,RemoteDevice* device);
private slots:
    void newDevice(QString name, QString ip, QString location);
    void readString();
    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpConnected();
    void tcpDisconnected();
    void processPendingDatagrams();
    void loadRowData(int row);
    void cancelButtonClicked();
    void continueButtonClicked();
    void repeatButtonClicked();

    void deviceListWidgetClicked(QListWidgetItem *);
    void deviceListWidgetDoubleClicked(QListWidgetItem *);

private:
    Ui::AutoSearchDialog *ui;
    void closeEvent(QCloseEvent *event);
    QString parseResponse(QNetworkReply *reply, QUrl &url);
};

#endif // AUTOSEARCHDIALOG_H
