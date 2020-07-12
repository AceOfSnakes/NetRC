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
