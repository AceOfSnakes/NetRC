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

#include <string>
#include <QDialog>
#include <QDebug>
#include <QModelIndex>
#include <QLibrary>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QSslSocket>
#include <QUdpSocket>
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
    void Connect(QString ip, int port);
    ~RemoteDevice();
    QString ip;
    int port;
    QTcpSocket* socket;
private slots:
    void _DataAvailable(){emit DataAvailable();}
    void _TcpError(QAbstractSocket::SocketError socketError){emit TcpError(socketError);}
    void _TcpConnected(){emit TcpConnected();}
    void _TcpDisconnected(){TcpDisconnected();}
signals:
    void DataAvailable();
    void TcpError(QAbstractSocket::SocketError socketError);
    void TcpConnected();
    void TcpDisconnected();
};

class AutoSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutoSearchDialog(QWidget *parent = 0, QString pingCommand="?RGD",
                              QString pingResponseStart="RGD", QString pingResponseStartOff="");
    ~AutoSearchDialog();

    int                     result;
    QString                 selectedFamily;
    QString                 selectedAddress;
    QString                 selectedDevice;
    int                     selectedPort;
    QVector<RemoteDevice*>  deviceInList;

protected:
    void changeEvent(QEvent *e);
    QString pingCommand;
    QString pingResponseStart;
    QString pingResponseStartOff;

    QMap<QString,RemoteDevice*>  remoteDevices;
    QVector<QUdpSocket*>    multicatsSockets;
    QHostAddress            groupAddress;
    QString removeDevice(QMap<QString,RemoteDevice*>  &remoteDevices,RemoteDevice* device);
    void SendMsg();
    void reconnect(QString & key,QString & ip,int port,RemoteDevice* device);
private slots:
    void NewDevice(QString name, QString ip, QString location);
    void ReadString();
    void TcpError(QAbstractSocket::SocketError socketError);
    void TcpConnected();
    void TcpDisconnected();
    void ProcessPendingDatagrams();
    void on_CancelButton_clicked();
    void on_continueButton_clicked();
    void on_repeatButton_clicked();
    void on_listWidget_clicked(const QModelIndex &index);
    void on_listWidget_doubleClicked(const QModelIndex &index);
    void loadRowData(int row);
//    void on_listWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::AutoSearchDialog *ui;
    void closeEvent(QCloseEvent *event);
};

#endif // AUTOSEARCHDIALOG_H
