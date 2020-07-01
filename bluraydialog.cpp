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
#include "bluraydialog.h"
//#include "settingsdialog.h"
//#include "ui_bluraydialog.h"
#include <QDebug>
#include <QSignalMapper>
#include <QtConcurrent/QtConcurrent>
#include <qtextcodec.h>
#include <QDateTime>
#include <QThread>
#include <QTimer>

BluRayDialog::BluRayDialog(QWidget *parent, QSettings &settings, PlayerInterface &Comm/*, SettingsDialog *settingsDialog*/) :
    QDialog(parent),
    m_Settings(settings),
//    m_SettingsDialog(settingsDialog),
//    ui(new Ui::BluRayDialog),
    m_PlayerInterface(Comm),
    m_PositionSet(false)
{
    signalMapper = new QSignalMapper(this);
    //    m_PlayerSettings
    //    m_Settings.unite(m_PlayerSettings);
    m_PlayerIpPort = 8102;
    m_PlayerOnline = false;

//    ui->setupUi(this);
    if (m_Settings.contains("PlayerSettings")) {
        m_PlayerInterface.reloadPlayerSettings(m_Settings.value("PlayerSettings").toMap());
    }
    // restore the position of the window
    if (m_Settings.value("SaveBlueRayWindowGeometry", false).toBool())
    {
        m_PositionSet = restoreGeometry(m_Settings.value("BlueRayWindowGeometry").toByteArray());
    }
    EnableControls(false);
//    ui->BdPowerButton->setEnabled(false);
    m_PowerButtonOnIcon.addFile ( ":/new/prefix1/images/Crystal_Clear_action_exit_green.png", QSize(128, 128));
    m_PowerButtonOffIcon.addFile ( ":/new/prefix1/images/Crystal_Clear_action_exit.png", QSize(128, 128));
    // player interface
    connect((&m_PlayerInterface), SIGNAL(Connected()), this, SLOT(CommConnected()));
    connect((&m_PlayerInterface), SIGNAL(Disconnected()), this, SLOT(CommDisconnected()));
    connect((&m_PlayerInterface), SIGNAL(CommError(QString)), this,  SLOT(CommError(QString)));
    connect((&m_PlayerInterface), SIGNAL(PlayerOffline(bool)), this,  SLOT(PlayerOffline(bool)));
    //connect((&m_PlayerInterface), SIGNAL(PlayerType(QString)), this,  SLOT(PlayerType(QString)));
    connect((&m_PlayerInterface), SIGNAL(SettingsChanged()), this,  SLOT(ChangeSettings()));
    connect((&m_PlayerInterface), SIGNAL(UpdateDisplayInfo(QRegExp &)), this,  SLOT(UpdateDisplayInfo(QRegExp &)));
    CheckOnline();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(CheckOnlineInternal()));
    timer->start(1000);
    this->setWindowTitle(tr("Blu-Ray player"));
    ChangeSettings();
    connect(signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(SendCmd(QString)));

}

void BluRayDialog::ChangeSettings() {
    //signalMapper->dumpObjectTree();
    //QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();
    //qDebug()<<signalMapper->dynamicPropertyNames();
    /*
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("Bd")||button->objectName().startsWith("Cursor")) {
            signalMapper->removeMappings(button);
            disconnect(button, SIGNAL(clicked()),signalMapper,SLOT(map()));
            if(!m_PlayerInterface.m_PlayerSettings.value(button->objectName()).isNull()) {
                signalMapper->setMapping(button,m_PlayerInterface.m_PlayerSettings.value(button->objectName()).toString());
                connect(button, SIGNAL(clicked())
                        ,signalMapper,SLOT(map()));
            }
        }
    }
    */
    m_Settings.setValue("PlayerSettings", m_PlayerInterface.m_PlayerSettings);
}

BluRayDialog::~BluRayDialog()
{
    delete signalMapper;
//    delete ui;
}

void BluRayDialog::moveEvent(QMoveEvent* event)
{
    m_Settings.setValue("BlueRayWindowGeometry", saveGeometry());
    QDialog::moveEvent(event);
}

void BluRayDialog::resizeEvent(QResizeEvent *event)
{
    m_Settings.setValue("BlueRayWindowGeometry", saveGeometry());
    QDialog::resizeEvent(event);
}

void BluRayDialog::ConnectPlayer()
{
//    ui->pushButtonConnect->setEnabled(false);
//    m_SettingsDialog->EnableIPInputBD(false);
    if (!m_PlayerInterface.IsConnected())
    {
        m_PlayerInterface.ConnectToPlayer(m_PlayerIpAddress, m_PlayerIpPort);
    }
}

void BluRayDialog::UpdateDisplayInfo (QRegExp &rx) {
    QString time = QString("--:--:--");
    QString track = QString("---:---");
    if(!rx.cap(2).isEmpty()) {
        time = rx.cap(3).append(":").append(rx.cap(4)).append(":").append(rx.cap(5));
        track.clear();
        if(!rx.cap(1).isEmpty()) {
            track.append(rx.cap(1)).append(":");
        }
        track.append(rx.cap(2));
    }
//    ui->BdTrackLabel->setText(track);
//    ui->BdTimeLabel->setText(time);
}

void BluRayDialog::PlayerOffline(bool offline) {
//    ui->BdPowerButton->setIcon((offline) ? m_PowerButtonOffIcon : m_PowerButtonOnIcon);
    if(offline) {
        this->setWindowTitle(tr("Blu-Ray player"));
        QRegExp qre;
        UpdateDisplayInfo(qre);
    }

    EnableControls(!offline);
    m_offline = offline;
}

void BluRayDialog::ManualShowBluRayDialog()
{
    ShowBluRayDialog(false);
}

void BluRayDialog::ShowBluRayDialog(bool autoShow)
{
    if ((!autoShow) || (m_Settings.value("AutoShowBlueRay", false).toBool() && !isVisible()))
    {
        m_Settings.dumpObjectTree();
        if (!m_PositionSet || !m_Settings.value("SaveBlueRayWindowGeometry", false).toBool())
        {
            QWidget* Parent = dynamic_cast<QWidget*>(parent());
            if (Parent != NULL)
            {
                int x = Parent->pos().x() + Parent->width() + 20;
                QPoint pos;
                pos.setX(x);
                pos.setY(Parent->pos().y());
                move(pos);
            }
        }
        show();
    }
}

void BluRayDialog::CommConnected()
{

//    ui->pushButtonConnect->setEnabled(true);
//    ui->pushButtonConnect->setText(tr("Disconnect"));
//    ui->pushButtonConnect->setChecked(true);
//    ui->BdPowerButton->setEnabled(true);
    m_PlayerOnline = true;
    EnableControls(true);
    CheckOnline();
}
void BluRayDialog::CommDisconnected()
{
    //ui->pushButtonConnect->setText(tr("Connect to Player"));
    //ui->pushButtonConnect->setEnabled(true);
    //ui->pushButtonConnect->setChecked(false);
    //m_SettingsDialog->EnableIPInputBD(true);
    EnableControls(false);
    //ui->BdPowerButton->setEnabled(false);
    m_PlayerOnline = false;
}
void BluRayDialog::CommError(QString/* socketError*/)
{
//    m_SettingsDialog->EnableIPInputBD(true);
//    ui->pushButtonConnect->setText(tr("Connect to Player"));
//    ui->pushButtonConnect->setEnabled(true);
//    ui->pushButtonConnect->setChecked(false);
//    m_SettingsDialog->EnableIPInputBD(true);
    EnableControls(false);
    m_PlayerOnline = false;
}

bool BluRayDialog::SendCmd(const QString& cmd)
{
    //qDebug()<<">>"+cmd;
    return m_PlayerInterface.SendCmd(cmd);
}

void BluRayDialog::EnableControls(bool enable)
{
//    QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();
/*
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("Bd")||button->objectName().startsWith("Cursor")) {
            if(m_PlayerInterface.m_PlayerSettings.contains(button->objectName())) {
                button->setEnabled(enable);
            } else {
                button->setEnabled(false);
            }
        }
    }
*/
}

void BluRayDialog::onConnect()
{
    if (!m_PlayerInterface.IsConnected())
    {
        // connect
        // read settings from the line edits
        QString ip1, ip2, ip3, ip4, ip_port;
        // first the 4 ip address blocks
//        m_SettingsDialog->GetIpAddressBD(ip1, ip2, ip3, ip4, ip_port);
        if (ip1 == "")
        {
            ip1 = "192"; // set default
        }
        if (ip2 == "")
        {
            ip2 = "168"; // set default
        }
        if (ip3 == "")
        {
            ip3 = "1"; // set default
        }
        if (ip4 == "")
        {
            ip4 = "192"; // set default
        }
        m_PlayerIpAddress = ip1 + "." + ip2 + "." + ip3 + "." + ip4;
        // and then th ip port
        if (ip_port == "")
        {
            ip_port = "8102"; // set default
            m_PlayerIpPort = 8102;
        }
        else
        {
            m_PlayerIpPort = ip_port.toInt();
        }
        // save the ip address and port permanently
//        m_SettingsDialog->SetIpAddressBD(ip1, ip2, ip3, ip4, ip_port);
        m_Settings.setValue("Player_IP/1", ip1);
        m_Settings.setValue("Player_IP/2", ip2);
        m_Settings.setValue("Player_IP/3", ip3);
        m_Settings.setValue("Player_IP/4", ip4);
        m_Settings.setValue("Player_IP/PORT", ip_port);
        ConnectPlayer();
    }
    else
    {
        // disconnect
        EnableControls(false);
        m_PlayerInterface.Disconnect();
        m_PlayerOnline = false;
    }
    CheckOnline();
}

void SendCommand(PlayerInterface&    m_PlayerInterface, QString &str) {
    qDebug()<<str;
    m_PlayerInterface.SendCmd(str);
}

void BluRayDialog::CheckOnlineInternal() {
    if(m_PlayerOnline) {
        foreach (QVariant ping_command, m_PlayerInterface.m_ping_commands) {
            SendCmd(ping_command.toString());
        }
    }
}

void BluRayDialog::CheckOnline() {
    m_offline=true;
    PlayerOffline(true);
}


void BluRayDialog::on_pushButtonConnect_clicked()
{
//    ui->pushButtonConnect->setChecked(!ui->pushButtonConnect->isChecked());
    onConnect();
}


void BluRayDialog::on_BdPowerButton_clicked()
{
    if (m_offline)
    {
        SendCmd(m_PlayerInterface.m_PlayerSettings.value("BdPowerOnButton").toString());
        CheckOnline();
    }
    else
    {
        SendCmd(m_PlayerInterface.m_PlayerSettings.value("BdPowerOffButton").toString());
        PlayerOffline(true);
    }
}


