/*
 * AVRPioRemote
 * Copyright (C) 2013  Andreas Müller, Ulrich Mensfeld
 *
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
#ifndef BLURAYDIALOG_H
#define BLURAYDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QSignalMapper>
#include <QListWidgetItem>
#include "playerinterface.h"
//#include "settingsdialog.h"
#include <QMoveEvent>
#include <QIcon>
#include <QMap>
#include <inttypes.h>
#include "receivedobjectbase.h"
namespace Ui {
class BluRayDialog;
}

class BluRayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BluRayDialog(QWidget *parent, QSettings& settings, PlayerInterface& Comm/*, SettingsDialog * settingsDialog*/);
    ~BluRayDialog();
    void ResponseReceived(ReceivedObjectBase *);
private:
    QSettings&          m_Settings;
//    SettingsDialog*     m_SettingsDialog;
    Ui::BluRayDialog*   ui;
    PlayerInterface&    m_PlayerInterface;
    bool                m_PositionSet;
    bool                m_offline;
    QIcon           m_PowerButtonOnIcon;
    QIcon           m_PowerButtonOffIcon;

    int             m_PlayerIpPort;
    QString         m_PlayerIpAddress;
    bool            m_PlayerOnline;
    QSignalMapper* signalMapper;


    void moveEvent(QMoveEvent*event);
    virtual void resizeEvent(QResizeEvent *event);

    void ConnectPlayer();
public slots:
    void ManualShowBluRayDialog();
    void ShowBluRayDialog(bool autoShow);
    void EnableControls(bool enable);
    void onConnect();
    void PlayerOffline(bool);
    void UpdateDisplayInfo (QRegExp &rx);
private slots:
    void CheckOnline();
    void CheckOnlineInternal();
    void CommConnected();
    void ChangeSettings();
    void CommDisconnected();
    void CommError(QString socketError);
    bool SendCmd(const QString& cmd);
    void on_pushButtonConnect_clicked();
    void on_BdPowerButton_clicked();
signals:
    //	    void SendCmd(QString data);
};

#endif // BLURAYDIALOG_H
