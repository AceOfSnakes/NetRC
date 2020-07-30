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
#include "remotecontrol.h"
#include "ui_remotecontrol.h"
#include <QDebug>
#include <QTimer>
#include <QSpacerItem>
#include <QWindowStateChangeEvent>
#include <QMenu>
#include <QFileDialog>
#include <QRect>
#include <QSettings>
#include <QJsonDocument>
#include <QMessageBox>
#include <QJsonObject>
#include <QNetworkAddressEntry>
#include <QWidget>
#include "aboutdialog.h"
#include "autosearchdialog.h"
#include "deviceconnector.h"
#include "rcsettings.h"

RemoteControl::RemoteControl(QWidget *parent) :
    QMainWindow(parent),
    settings(),
    regx(".*"),
    deviceInterface(),
    ui(new Ui::RemoteControl)
{
    signalMapper = new QSignalMapper(this);

    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint) &(~Qt::WindowMaximizeButtonHint));
    ui->setupUi(this);
    ui->mainWidget->setVisible(true);
    /*
    addPanel(QString("Custom Panel 1"));
    addPanel(QString("Custom Panel 2"));
    addPanel(QString("Custom Panel 3"));
    addPanel(QString("Custom Panel 4"));
    */
    loadSettings();
    m_tray_icon = new QSystemTrayIcon(QIcon(QString(":/images/").append("icon-power").append(".png")), this);

    connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_show_hide(QSystemTrayIcon::ActivationReason)));

    QAction *quit_action = new QAction( "Exit", m_tray_icon );
    connect( quit_action, SIGNAL(triggered()), this, SLOT(quit()) );

    QAction *hide_action = new QAction( "Show / Hide", m_tray_icon );
    connect( hide_action, SIGNAL(triggered()), this, SLOT(on_show_hide()) );

    QMenu *tray_icon_menu = new QMenu;
    tray_icon_menu->addAction( hide_action );
    tray_icon_menu->addAction( quit_action );

    powerButtonOnIcon.addFile( ":/images/power_green.png", QSize(128, 128));
    powerButtonOffIcon.addFile( ":/images/power_red.png", QSize(128, 128));

    connectButtonOnIcon.addFile( ":/images/connect-green.png", QSize(128, 128));
    connectButtonOffIcon.addFile( ":/images/connect-red.png", QSize(128, 128));


    m_tray_icon->setContextMenu( tray_icon_menu );
    if(ui->actionMinimize_To_Tray->isChecked()) {
        m_tray_icon->show();
    }
    ui->btn_Connect->setEnabled(true);
    connect(ui->actionMinimize_To_Tray,SIGNAL(changed()), this, SLOT(on_MinimizeToTrayChanged()));


    connect((&deviceInterface), SIGNAL(deviceConnected()), this, SLOT(commConnected()));
    connect((&deviceInterface), SIGNAL(deviceDisconnected()), this, SLOT(commDisconnected()));
    connect((&deviceInterface), SIGNAL(commError(QString)), this,  SLOT(commError(QString)));
    connect((&deviceInterface), SIGNAL(deviceOffline(bool)), this,  SLOT(deviceOffline(bool)));
    connect((&deviceInterface), SIGNAL(settingsChanged()), this,  SLOT(changeSettings()));
    connect((&deviceInterface), SIGNAL(updateDisplayInfo(QRegExp &)), this,  SLOT(updateDisplayInfo(QRegExp &)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkOnlineInternal()));
    timer->start(1000);


    reloadMenu();
    show();
    ui->btn_Connect->setEnabled(true);
    connect(signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(sendCmd(QString)));

    redraw();
    //ui->listView->row().
    qDebug()<<"ui->listWidget->item(1)->font().pixelSize()"<< ui->listWidget->item(1)->font().weight();
}

void RemoteControl::reloadMenu() {
    QStringList saved = RCSettings::settingsList();
    saved.sort();
}

RemoteControl::~RemoteControl() {
    delete ui;
}

void RemoteControl::redraw() {
    ui->actionMenu->setChecked(ui->menuWidget->isVisible());
    ui->actionNumbers->setChecked(ui->numWidget->isVisible());
    ui->actionNavigate->setChecked(ui->navigateWidget->isVisible());
    ui->actionColor->setChecked(ui->colorWidget->isVisible());

    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(QRegularExpression("btn.*"), Qt::FindChildrenRecursively)) {
      action->setEnabled(ui->actionView_Enabled->isChecked());
    }
    ui->btn_Connect->setEnabled(true);

    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(regx, Qt::FindDirectChildrenOnly)) {
        if(!action->objectName().isEmpty()) {
            foreach(QAction *ac, ui->menuView_2->findChildren<QAction*>(QRegularExpression("action" + action->objectName()), Qt::FindDirectChildrenOnly) ){
                ac->setChecked(action->isVisible());
            }
        }
    }
}

void RemoteControl::initSetting(const QString & set, QVariant & value) {
    QSettings sets(qApp->organizationName(),qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    if (sets.value(set).type() == QVariant::Invalid) {
        sets.setValue(set, value);
    }
    sets.endGroup();
    sets.endGroup();
}

void RemoteControl::loadSettings() {

    QVariant truex(true);

    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    ui->actionView_Enabled->setChecked(sets.value("showEnabled").toBool());
    ui->actionMinimize_To_Tray->setChecked(sets.value("minimizeToTray").toBool());
    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(regx, Qt::FindDirectChildrenOnly)) {
        initSetting(action->objectName(), truex);
        action->setVisible(sets.value(action->objectName()).toBool());
    }
    sets.endGroup();
    sets.endGroup();
}

void RemoteControl::saveSettings() {
    QSettings sets(qApp->organizationName(),qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    sets.setValue("showEnabled", QVariant(ui->actionView_Enabled->isChecked()));
    sets.setValue("minimizeToTray", QVariant(ui->actionMinimize_To_Tray->isChecked()));
    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(regx, Qt::FindDirectChildrenOnly)) {
        if(!action->objectName().isEmpty()) {
            sets.setValue(action->objectName(), action->isVisible());
        }
    }
    sets.endGroup();
    sets.endGroup();
}

void RemoteControl::on_actionAbout_triggered(){
    AboutDialog about(this);
    about.exec();
}

void RemoteControl::showWidget(QWidget *widget, bool flag) {
    widget->setVisible(flag);
    widget->setEnabled(ui->actionView_Enabled->isChecked());
    saveSettings();
}

void RemoteControl::on_actionNumbers_triggered() {
    showWidget(ui->numWidget,ui->actionNumbers->isChecked());
}

void RemoteControl::on_actionMenu_triggered() {
    showWidget(ui->menuWidget,ui->actionMenu->isChecked());
}

void RemoteControl::on_actionColor_triggered() {
    showWidget(ui->colorWidget,ui->actionColor->isChecked());
}

void RemoteControl::on_actionNavigate_triggered() {
    showWidget(ui->navigateWidget,ui->actionNavigate->isChecked());
}

void RemoteControl::on_actionView_Enabled_triggered() {
    saveSettings();
    redraw();
}

void RemoteControl::switchPanel() {
    QAction *action = qobject_cast<QAction *> (sender());
    if (action) {
        qDebug() << "action" << action->objectName();
        foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(QRegularExpression(action->objectName().replace("action","")), Qt::FindDirectChildrenOnly)) {
            action->setVisible(!action->isVisible());
        }
        saveSettings();
    }
}


void RemoteControl::addPanel(QString panelName) {
    QWidget *widInt = new QWidget(ui->centralWidget);
    widInt->setEnabled(true);
    widInt->setVisible(true);
    widInt->setObjectName(panelName);
    QGridLayout* layout = new QGridLayout(widInt);
    layout->setMargin(2);
    layout->setSpacing(0);

    int row , column = 0;
    widInt->setLayout(layout);
    QFrame *vline = new QFrame();
    vline->setFrameShape(QFrame::HLine);
    vline->setFrameShadow(QFrame::Sunken);
    for (row = 0; row < 2; row++) {
        for (column = 0; column < 4; column++) {
            QPushButton *btn = new QPushButton(QString().asprintf("X%d/%d", row, column));
            btn->setObjectName(QString().asprintf("btnX%d/%d", row, column));
            btn->setStyleSheet("font: bold");
            layout->addWidget(btn, row, column);
        }
    }
    ui->centralWidget->layout()->addWidget(widInt);
    QAction *menuAction = new QAction(panelName, ui->menuView_2);

    menuAction->setCheckable(true);
    //menuAction->setObjectName("action" + panelName);
    //ui->menuView_2->insertAction(ui->actionColor, menuAction);
    connect(menuAction, SIGNAL(triggered()), this, SLOT(switchPanel()));
    QVariant truex(true);
    initSetting(panelName, truex);
}


void RemoteControl::on_MinimizeToTrayChanged() {
    on_show_hide( );
    saveSettings();
}

void RemoteControl::on_show_hide( ) {
    if(ui->actionMinimize_To_Tray->isChecked()) {
        isVisible() ? hide() : show();
    }
}

void RemoteControl::on_show_hide(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        on_show_hide();
    default:
        break;
    }
}

void RemoteControl::changeEvent(QEvent *e) {
    e->accept();
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::WindowStateChange:
    case QEvent::ActionChanged: {
        on_show_hide();
        break;
    }
    default:
        break;
    }
}

void RemoteControl::quit() {
    this->close();
}

void RemoteControl::on_btn_Connect_clicked() {
    if(deviceInterface.isConnected()) {
        deviceInterface.disconnect();
    } else {
        qDebug()<<"Connect to"<<deviceIpAddress<<deviceIpPort;
        if(deviceIpPort != -1) {
            deviceInterface.connectToDevice(deviceIpAddress, deviceIpPort);
        } else {
            on_btn_Connect_customContextMenuRequested(QPoint());
        }
    }
}

void RemoteControl::on_btn_Connect_customContextMenuRequested(const QPoint &/*pos*/)
{
    DeviceConnector deviceConnector(settings, this);
    deviceConnector.setDevice(deviceName, deviceIpAddress, deviceIpPort);
    qDebug() << deviceName << deviceIpAddress << deviceIpPort;
    deviceConnector.exec();
    qDebug () << deviceConnector.deviceFamily;
    if(!deviceConnector.deviceFamily.isEmpty()) {
        ui->listWidget->item(1)->setText(deviceConnector.deviceFamily);
        ui->listWidget->item(2)->setText(deviceConnector.device);
        ui->listWidget->item(3)->setText(deviceConnector.deviceAddress);
        deviceName = deviceConnector.device;
        settings.swap(deviceConnector.settings);
        deviceInterface.reloadDeviceSettings(settings.toMap());
        deviceInterface.connectToDevice(deviceConnector.deviceIPAddress, deviceConnector.devicePort);
        deviceIpAddress = deviceConnector.deviceIPAddress;
        deviceIpPort = deviceConnector.devicePort;
        checkOnline();
    }
}



// MIGRATION
void RemoteControl::connectDevice()
{
    // TODO ui->pushButtonConnect->setEnabled(false);
    // TODO m_SettingsDialog->EnableIPInputBD(false);
    if (!deviceInterface.isConnected()) {
        deviceInterface.connectToDevice(deviceIpAddress, deviceIpPort);
    }
}

void RemoteControl::updateDisplayInfo (QRegExp &rx) {
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
    ui->listWidget->item(5)->setText(QString("Track ").append(track).append(" Time ").append(time));
}

void RemoteControl::deviceOffline(bool offline) {
    qDebug()<<"DeviceOffline(bool offline);"<<offline;
    ui->btn_Power->setIcon((offline) ? powerButtonOffIcon : powerButtonOnIcon);
    if(offline) {
        QRegExp qre;
        updateDisplayInfo(qre);
    }
    enableControls(!offline);
    //deviceOnline = !offline;
    offlineStatus = offline;
}

void RemoteControl::enableControls(bool enable)
{
    QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("rc_btn_")) {
            QString btnName = button->objectName().replace("rc_btn_","");
            if(deviceInterface.deviceSettings.contains(btnName)) {
                button->setEnabled(enable);
            } else {
                button->setEnabled(false);
            }
        }
    }
}

void RemoteControl::onConnect()
{
    qDebug()<<"deviceInterface.IsConnected()"<<deviceInterface.isConnected();
    if (!deviceInterface.isConnected()) {
        // connect
        // read settings from the line edits
        QString ip1, ip2, ip3, ip4, ip_port;
        // first the 4 ip address blocks
        // TODO m_SettingsDialog->GetIpAddressBD(ip1, ip2, ip3, ip4, ip_port);
        if (ip1 == "") {
            ip1 = "192"; // set default
        }
        if (ip2 == "") {
            ip2 = "168"; // set default
        }
        if (ip3 == "") {
            ip3 = "1"; // set default
        }
        if (ip4 == "") {
            ip4 = "192"; // set default
        }
        deviceIpAddress = ip1 + "." + ip2 + "." + ip3 + "." + ip4;
        // and then the ip port
        if (ip_port == "") {
            ip_port = "8102"; // set default
        }
        deviceIpPort = ip_port.toInt();
        // save the ip address and port permanently
        // TODO m_SettingsDialog->SetIpAddressBD(ip1, ip2, ip3, ip4, ip_port);
        // TODO m_Settings.setValue("Player_IP/1", ip1);
        // TODO m_Settings.setValue("Player_IP/2", ip2);
        // TODO m_Settings.setValue("Player_IP/3", ip3);
        // TODO m_Settings.setValue("Player_IP/4", ip4);
        // TODO m_Settings.setValue("Player_IP/PORT", ip_port);

        connectDevice();

    }
    else {
        // disconnect
        enableControls(false);
        deviceInterface.disconnect();
        deviceOnline = false;
    }
    checkOnline();
}

void RemoteControl::checkOnlineInternal() {
    if(deviceOnline) {
        foreach (QVariant ping_command, deviceInterface.pingCommands) {
            sendCmd(ping_command.toString());
        }
    }
}

void RemoteControl::checkOnline() {
    offlineStatus = true;
    deviceOffline(true);
}

void RemoteControl::commConnected() {
    ui->btn_Connect->setToolTip(tr("\n    Disconnect Device: ").append(deviceName).append(" ")
                                .append(deviceIpAddress).append(QString().asprintf(":%d    ", deviceIpPort))
                                .append("\n\n    Right click to connect to another device    \n"));
    ui->btn_Connect->setText(tr("Disconnect"));
    ui->btn_Connect->setChecked(true);
    ui->btn_Connect->setEnabled(true);
    //ui->btn_Connect->adjustSize();
    ui->btn_Power->setEnabled(true);
    deviceOnline = true;
    enableControls(true);
    ui->btn_Connect->setIcon(connectButtonOnIcon);
    checkOnline();
}

void RemoteControl::commDisconnected() {
    ui->btn_Connect->setToolTip(tr("\n    Connect To Device: ").append(deviceName).append(" ")
                                .append(deviceIpAddress).append(QString().asprintf(":%d    \n", deviceIpPort)));
    ui->btn_Connect->setText(tr("Connect"));
    ui->btn_Connect->setEnabled(true);
    ui->btn_Connect->setChecked(false);
    // TODO m_SettingsDialog->EnableIPInputBD(true);
    enableControls(false);
    ui->btn_Power->setEnabled(false);
    ui->btn_Connect->setIcon(connectButtonOffIcon);
    deviceOnline = false;
}

void RemoteControl::commError(QString/* socketError*/) {
    commDisconnected();
}

bool RemoteControl::sendCmd(const QString& cmd) {
    return deviceInterface.sendCmd(cmd);
}

void RemoteControl::changeSettings() {
    QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("rc_btn_")) {
            signalMapper->removeMappings(button);
            disconnect(button, SIGNAL(clicked()),signalMapper,SLOT(map()));
            QString btnName = button->objectName().replace("rc_btn_","");
            if(!deviceInterface.deviceSettings.value(btnName).isNull()) {
                signalMapper->setMapping(button,deviceInterface.deviceSettings.value(btnName).toString());
                connect(button, SIGNAL(clicked())
                        ,signalMapper,SLOT(map()));
            }
        }
    }
    // TODO m_Settings.setValue("PlayerSettings", m_PlayerInterface.m_PlayerSettings);
}


void RemoteControl::on_btn_Power_clicked()
{
    qDebug()<<"deviceOnline"<<deviceOnline<<"OFFLINE STATUS"<<offlineStatus;
    if (offlineStatus ) {
        sendCmd(deviceInterface.deviceSettings.value("powerOn").toString());
//        CheckOnline();
    } else {
        sendCmd(deviceInterface.deviceSettings.value("powerOff").toString());
//        DeviceOffline(true);
    }
}

