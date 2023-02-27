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
#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QMainWindow>
#include <QRegularExpression>
#include <QSignalMapper>
#include <QSystemTrayIcon>
#include "deviceinterface.h"
#include "debug.h"

namespace Ui {
class RemoteControl;
}

class RemoteControl : public QMainWindow
{
    Q_OBJECT
    QMap<QString,QString> defaultTooltips;
    QMap<QString,QString> defaultLabels;
    QVariant              settings;
    QRegularExpression    regx;
    Debug                 *debugDialog = nullptr;

    DeviceInterface    deviceInterface;
    bool               offlineStatus;
    bool               deviceOnline;
    QSignalMapper*     signalMapper;
    unsigned int       deviceIpPort = 0;
    QString            deviceIpAddress;
    QString            deviceFamily;
    QString            deviceName;

    void connectDevice();

    QIcon           powerButtonOnIcon;
    QIcon           powerButtonOffIcon;
    QIcon           connectButtonOnIcon;
    QIcon           connectButtonOffIcon;

    QIcon           trayGray;
    QIcon           trayGreen;
    QIcon           trayRed;
    QMap<QString,QList<QPushButton *>> buttonGroups;
    QSet<QPushButton *> enabledButtons;
public:
    explicit RemoteControl(QWidget *parent = 0);
    ~RemoteControl();
    void reconnect();
    void reloadPanels();
    void removeEmpty();
    void restoreSettings();
    void initConnect();
    void initTray();

protected:
     void changeEvent(QEvent *e);
     void closeEvent(QCloseEvent *event);

private slots:
    void closeDebug();
    void switchPanel();
    void deviceOffline(bool);
    void updateDisplayInfo (QRegularExpressionMatch rx);
    void updateDeviceInfo ();
    void enableControls(bool enable);
    void checkOnline();
    void checkOnlineInternal();
    void commConnected();
    void changeSettings();
    void commDisconnected();
    void commError(QString socketError);
    bool sendCmd(const QString& cmd);
    void onConnect();
    void newDevice();
    void reloadAndReconnect();
    void reloadAndReconnect(QString);
    void setEnableDevMode(bool);
    void debugClicked();
    void about();
    void quit();
    void settingsClicked();
    void connectClicked();
    void powerClicked();
    void showHideWithReason(QSystemTrayIcon::ActivationReason reason);
    void showHide();
    void connectCustomMenuRequested(const QPoint &pos);
    void hideAll();
    void showAll();
    void showMinimizedAll();
    void specialControl(const QString control, bool enabled);
    void changeTheme(QByteArray style);
private:
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    Ui::RemoteControl *ui;
    QSystemTrayIcon* trayIcon;
    void showWidget(QWidget *widget, bool flag);
    void select(const QVariant action);
    void redraw();
    void loadSettings();
    void reloadLatestDevice();
    void initSetting(const QString & set, QVariant & value);
    void addPanel(int pnl_idx, const QJsonArray &buttons);
    void reloadMenu();
    void clearInformationPanel();
    bool eventFilter(QObject *obj, QEvent *event);
signals:
    void deviceActivated();
};

#endif // REMOTECONTROL_H
