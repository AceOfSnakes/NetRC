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

namespace Ui {
class RemoteControl;
}

class RemoteControl : public QMainWindow
{
    Q_OBJECT
    QVariant settings;
    QRegularExpression regx;//(".*");
    // MIGRATION
    DeviceInterface deviceInterface;
    void ConnectPlayer();
    bool            offlineStatus;
    bool            deviceOnline;
    QSignalMapper*  signalMapper;
    int             deviceIpPort;
    QString         deviceIpAddress;

    // MIGRATION END
public:
    explicit RemoteControl(QWidget *parent = 0);
    ~RemoteControl();
protected:
     void changeEvent(QEvent *e);
private slots:
    void on_actionAbout_triggered();
    void on_actionNumbers_triggered();
    void on_actionMenu_triggered();
    void on_actionColor_triggered();
    void on_actionNavigate_triggered();
    void on_actionView_Enabled_triggered();
    void switchPanel();
    void on_MinimizeToTrayChanged();
    void on_show_hide(QSystemTrayIcon::ActivationReason reason);
    void on_show_hide();
    void quit();
//  void on_actionLoad_Configuration_triggered();
//    void onLoadFamily();

    void on_btn_Connect_clicked();

    void on_btn_Connect_customContextMenuRequested(const QPoint &pos);
    // MIGRATION
    void PlayerOffline(bool);
    void UpdateDisplayInfo (QRegExp &rx);
    void EnableControls(bool enable);
    void CheckOnline();
    void CheckOnlineInternal();
    void CommConnected();
    void ChangeSettings();
    void CommDisconnected();
    void CommError(QString socketError);
    bool SendCmd(const QString& cmd);
    void onConnect();
    // MIGRATION END
private:
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    Ui::RemoteControl *ui;
    QSystemTrayIcon* m_tray_icon;
    void showWidget(QWidget *widget, bool flag);
    void saveSettings();
    void select(const QVariant action);
    void redraw();
    void loadSettings();
    void initSetting(const QString & set, QVariant & value);
    void addPanel(QString style);
    void reloadMenu();
};

#endif // REMOTECONTROL_H
