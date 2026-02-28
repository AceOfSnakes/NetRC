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
#include "crypto.h"

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
    void _tcpError(QAbstractSocket::SocketError socketError){
        emit tcpError(socketError);
    }
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
private:
    QString circle = "⬤";
    Crypto::CryptoSettings cryptoSettings;
    bool cryptoEnabled = false;
public:
    enum DebugColor { inboundColor, outboundColor , alertColor, informationColor};
    QMap<DebugColor, QColor> mapColored {
        {inboundColor, QColorConstants::Svg::lightgreen},
        {outboundColor, QColorConstants::Svg::lightblue},
        {informationColor, QColorConstants::Svg::lightgray},
        {alertColor, QColorConstants::Svg::tomato}
    };

    Q_ENUM(DebugColor);
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
    Ui::AutoSearchDialog *ui;

    void applyCryptoSettings(Crypto::CryptoSettings sets) {
        cryptoSettings = sets;
        if(cryptoEngine != nullptr) {
            delete cryptoEngine;
            cryptoEngine = nullptr;
        }
        cryptoEngine = new Crypto(cryptoSettings);
        qDebug() << "AutoSearchDialog cryptoSettings" << cryptoSettings;
        cryptoEnabled = true;
    }

    void resetCryptoSettings() {
        if(cryptoEngine != nullptr) {
            delete cryptoEngine;
            cryptoEngine = nullptr;
        }
        cryptoEnabled = false;
    }

public slots:
    void showDebug(const QString &value, const AutoSearchDialog::DebugColor &);

protected:
    void changeEvent(QEvent *e);
    QString pingCommand;
    QString pingResponseStart;
    QString pingResponseStartOff;
    Crypto  *cryptoEngine = nullptr;
    QMap<QString,RemoteDevice*>  remoteDevices;
    QVector<QUdpSocket*>         multicatsSockets;
    QHostAddress                 groupAddress;

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

    void closeEvent(QCloseEvent *event);
    QString parseResponse(QNetworkReply *reply, QUrl &url);

signals:
    void processResponse(const QString &response);
    void processError(const QString &response);
    void processRequest(const QString &response);
};

Q_DECLARE_METATYPE(AutoSearchDialog::DebugColor);
#endif // AUTOSEARCHDIALOG_H
