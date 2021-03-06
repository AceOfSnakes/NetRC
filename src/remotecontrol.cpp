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
#include <QMetaType>
#include <QTimer>
#include <QScreen>
#include <QSpacerItem>
#include <QStyleFactory>
#include <QWindowStateChangeEvent>
#include <QMenu>
#include <QFileDialog>
#include <QRect>
#include <QSettings>
#include <QJsonDocument>
#include <QMessageBox>
#include <QJsonObject>
#include <QSpacerItem>
#include <QJsonArray>
#include <QNetworkAddressEntry>
#include <QWidget>
#include "aboutdialog.h"
#include "autosearchdialog.h"
#include "debug.h"
#include "deviceconnector.h"
#include "rcsettings.h"
#include "settings.h"

void RemoteControl::restoreSettings()
{
    QSettings settings(qApp->organizationName(),
                       qApp->applicationName());

    settings.beginGroup("global");
    setGeometry(settings.value("geometryX").toInt(),settings.value("geometryY").toInt(),0,0);
    QByteArray style = settings.value("theme").toByteArray();

    if(style.isEmpty()) {
        QFile file(":/style.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    } else  {
        QString styleSheet = QLatin1String(style);
        this->setStyleSheet(styleSheet);
    }

    settings.endGroup();
}

void RemoteControl::initConnect()
{
    // Device interface
    connect((&deviceInterface), SIGNAL(deviceConnected()), this, SLOT(commConnected()));
    connect((&deviceInterface), SIGNAL(deviceDisconnected()), this, SLOT(commDisconnected()));
    connect((&deviceInterface), SIGNAL(commError(QString)), this,  SLOT(commError(QString)));
    connect((&deviceInterface), SIGNAL(deviceOffline(bool)), this,  SLOT(deviceOffline(bool)));
    connect((&deviceInterface), SIGNAL(settingsChanged()), this,  SLOT(changeSettings()));
    connect((&deviceInterface), SIGNAL(updateDisplayInfo(QRegularExpressionMatch)),
            this,  SLOT(updateDisplayInfo(QRegularExpressionMatch)));
    connect((&deviceInterface), SIGNAL(updateDeviceInfo()), this,  SLOT(updateDeviceInfo()));

    // Signal mapper
    connect(signalMapper, SIGNAL(mappedString(const QString&)), this, SLOT(sendCmd(QString)));

    // Timer for checking online device status
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkOnlineInternal()));
    timer->start(1000);

    // Buttons
    connect(ui->debugButton, SIGNAL(clicked()), this, SLOT(debugClicked()));
    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->minButton, SIGNAL(clicked()), this, SLOT(showHide()));
    connect(ui->aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui->connectButton, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(connectCustomMenuRequested(QPoint)));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(settingsClicked()));
    connect(ui->powerButton, SIGNAL(clicked()), this, SLOT(powerClicked()));
}

void RemoteControl::initTray() {

    trayIcon = new QSystemTrayIcon(QIcon(QString(":/images/").append("about").append(".png")), this);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &RemoteControl::showHideWithReason);

    QAction *quitAction = new QAction( "Exit", trayIcon );
    connect( quitAction, SIGNAL(triggered()), this, SLOT(close()) );

    QAction *hideAction = new QAction( "Show / Hide", trayIcon );
    connect( hideAction, SIGNAL(triggered()), this, SLOT(showHide()) );

    QMenu *trayIconMenu = new QMenu;
    trayIconMenu->addAction( hideAction );
    trayIconMenu->addAction( quitAction );

    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(close()));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

}

RemoteControl::RemoteControl(QWidget *parent) :
    QMainWindow(parent),
    settings(),
    regx(".*"),
    deviceInterface(),
    ui(new Ui::RemoteControl)
{

    defaultLabels.insert("channel", "Ch");
    defaultLabels.insert("volume", "Vol");

    signalMapper = new QSignalMapper(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->setupUi(this);

    restoreSettings();
    ui->mainWidget->setVisible(true);
    loadSettings();

    initTray();
    powerButtonOnIcon.addFile( ":/images/power_green.png", QSize(128, 128));
    powerButtonOffIcon.addFile( ":/images/power_red.png", QSize(128, 128));

    connectButtonOnIcon.addFile( ":/images/connect-green.png", QSize(128, 128));
    connectButtonOffIcon.addFile( ":/images/connect-red.png", QSize(128, 128));
    if(!RCSettings::isDevelopmentModeEnabled()) {
        setEnableDevMode(false);
    }
    initConnect();
    reloadMenu();
    show();
    ui->connectButton->setEnabled(true);
    redraw();
    reloadLatestDevice();
    qApp->installEventFilter(this);
}

void RemoteControl::reloadMenu() {
    QStringList saved = RCSettings::settingsList();
    saved.sort();
}

RemoteControl::~RemoteControl() {
    qApp->removeEventFilter(this);
    delete ui;
}

void RemoteControl::redraw() {
    ui->actionMenu->setChecked(ui->menuWidget->isVisible());
    ui->actionNumbers->setChecked(ui->numWidget->isVisible());
    ui->actionNavigate->setChecked(ui->navigateWidget->isVisible());
    ui->actionColor->setChecked(ui->colorWidget->isVisible());

    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(
                QRegularExpression("btn.*"),
                Qt::FindChildrenRecursively)) {
        action->setEnabled(ui->actionView_Enabled->isChecked());
    }
    ui->connectButton->setEnabled(true);
}

void RemoteControl::initSetting(const QString & set, QVariant & value) {
    QSettings sets(qApp->organizationName(),qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    if (!sets.value(set).isValid()) {
        sets.setValue(set, value);
    }
    sets.endGroup();
    sets.endGroup();
}

void RemoteControl::loadSettings() {

    QVariant truex(true);
    foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(
                regx,
                Qt::FindDirectChildrenOnly)) {
        initSetting(action->objectName(), truex);
    }
}


void RemoteControl::showWidget(QWidget *widget, bool flag) {
    widget->setVisible(flag);
    widget->setEnabled(ui->actionView_Enabled->isChecked());
}

void RemoteControl::switchPanel() {
    QAction *action = qobject_cast<QAction *> (sender());
    if (action) {
        foreach(QWidget *action, ui->centralWidget->findChildren<QWidget*>(
                    QRegularExpression(action->objectName().replace("action","")),
                    Qt::FindDirectChildrenOnly)) {
            action->setVisible(!action->isVisible());
        }
    }
}

void RemoteControl::addPanel(int panelIdx, const QJsonArray &buttons) {

//    ui->centralWidget->layout()->addWidget(
//                new QSpacerItem(0,1000, QSizePolicy::Expanding, QSizePolicy::Expanding));
    QWidget *widInt = new QWidget(ui->centralWidget);
    widInt->setEnabled(true);
    widInt->setVisible(true);
    QString panelName = QString().asprintf("Panel_%d", panelIdx);
    widInt->setObjectName(panelName);
    QGridLayout* layout = new QGridLayout(widInt);
//    layout->addWidget(
//                    new QSpacerItem(0,1000, QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->setContentsMargins(9, 6, 9, 0);
    layout->setSpacing(3);

    uint row , column = 0;
    widInt->setLayout(layout);
    QFrame *vline = new QFrame();
    vline->setFrameShape(QFrame::HLine);
    vline->setFrameShadow(QFrame::Sunken);


    int  offset = 0;
    uint buttons_count = 3;
    // Check panel settings
    QMap<QString, QVariant>sets = buttons.at(0).toVariant().toMap();
    uint columns = sets.value("columns").toUInt();
    uint panelPosition = sets.value("position").toUInt();
    if(columns > 0 || panelPosition > 0) {
        buttons_count = columns;
        // skip first button as settings element
        offset = 1;
    }
    uint rows = (buttons.size() + buttons_count - 1 - offset) / buttons_count;
    for (row = 0; row < rows; row++) {
        for (column = 0; column < buttons_count; column++) {
            QMap<QString, QVariant>button = buttons.at(
                        offset + (row * buttons_count) + column).toVariant().toMap();
            QString title = button.value("title").toString();
            if(!title.isEmpty()) {
                QPushButton *btn = new QPushButton(title);
                btn->setObjectName(QString().asprintf("rc_btn_").append(button.value("btn").toString()));
                btn->setMinimumWidth(50);
                btn->setMinimumHeight(28);
                btn->setMaximumHeight(28);
                btn->setEnabled(false);
                layout->addWidget(btn, row, column);
            }
        }
    }

    QFrame * lineA = new QFrame();
    lineA->setFrameShape(QFrame::HLine);
    lineA->setFrameShadow(QFrame::Sunken);
    QSpacerItem *item = new QSpacerItem(0, 0);
    layout->addItem(item, rows, 0, 1, buttons_count);
    layout->addWidget(lineA, rows + 1, 0, 1, buttons_count);

    ((QBoxLayout*)ui->centralWidget->layout())->insertWidget(panelPosition > 0? panelPosition: 1, widInt);


    QVariant truex(true);

    initSetting(panelName, truex);
}

void RemoteControl::showHide( ) {
    if (RCSettings::isMinimizeToTrayEnabled() ) {
        isVisible() ? hide() : show(); activateWindow();
    }
}

void RemoteControl::showHideWithReason(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showHide();
    default:
        break;
    }
}

void RemoteControl::changeEvent(QEvent *e) {
    //e->accept();
    //QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::WindowStateChange:
        showHide();
        //e->accept();
        break;
    default:
        break;
    }
}

void RemoteControl::connectClicked() {
    if(deviceInterface.isConnected()) {
        deviceInterface.disconnect();
    } else {
        if(deviceIpPort != 0) {
            deviceInterface.connectToDevice(deviceIpAddress, deviceIpPort);
        } else {
            connectCustomMenuRequested(QPoint());
        }
    }
}
void RemoteControl::newDevice() {
    DeviceConnector deviceConnector(settings, this);
    deviceConnector.setDevice(deviceFamily, deviceName, deviceIpAddress, deviceIpPort);

    deviceConnector.exec();

    if(!deviceConnector.deviceFamily.isEmpty()) {
        deviceName = deviceConnector.device;
        settings.swap(deviceConnector.settings);
        deviceIpAddress = deviceConnector.deviceIPAddress;
        deviceFamily = deviceConnector.deviceFamily;
        deviceIpPort = deviceConnector.devicePort;
        reconnect();
        checkOnline();
    }

}
void RemoteControl::connectCustomMenuRequested(const QPoint &pos)
{
    QList<QString> devices;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("devices");
    devices = sets.childGroups();
    sets.endGroup();
    sets.endGroup();


    if(devices.isEmpty()) {
        newDevice();
    }
    else {
        QMenu menu(ui->connectButton);
        foreach(QString device, devices) {
            menu.addAction(QString(device).replace("@@","/"), this, SLOT(reloadAndReconnect()));
        }
        menu.addSeparator();
        menu.addAction("&Device Management", this, SLOT(newDevice()));
        menu.exec(this->mapToGlobal(pos));
    }
}

void RemoteControl::connectDevice()
{
    if (!deviceInterface.isConnected()) {
        deviceInterface.connectToDevice(deviceIpAddress, deviceIpPort);
    }
}
void RemoteControl::updateDeviceInfo () {
    ui->statusDisplayWidget->item(7)->setText(deviceInterface.deviceName);
}

void RemoteControl::updateDisplayInfo (QRegularExpressionMatch rx) {
    QString time = QString("--:--:--");
    QString track = QString("---:---");

    if(!rx.captured(2).isEmpty()) {
        time = rx.captured(3).append(":").append(rx.captured(4)).append(":").append(rx.captured(5));
        track.clear();
        if(!rx.captured(1).isEmpty()) {
            track.append(rx.captured(1)).append(":");
        }
        track.append(rx.captured(2));
    }
    ui->statusDisplayWidget->item(5)->setText(QString("Track ").append(track).append(" Time ").append(time));

}

void RemoteControl::deviceOffline(bool offline) {
    ui->powerButton->setIcon((offline) ? powerButtonOffIcon : powerButtonOnIcon);
    enableControls(!offline);
    offlineStatus = offline;
}

void RemoteControl::enableControls(bool enable)
{
    QList<QPushButton *> allPButtons =
            ui->centralWidget->findChildren<QPushButton*>(
                QRegularExpression("rc_btn.*"), Qt::FindChildrenRecursively);
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("rc_btn_")) {
            QString btnName = button->objectName().replace("rc_btn_","");
            if(deviceInterface.deviceSettings.contains(btnName)) {
                button->setEnabled(enable);
            } else {
                button->setEnabled(false);
                button->setStyleSheet("");
            }
        }
    };
}

void RemoteControl::onConnect() {
    if (!deviceInterface.isConnected()) {
        // connect
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
        if(!deviceInterface.deviceSettings.value("initCmd").isNull()) {
            sendCmd(deviceInterface.deviceSettings.value("initCmd").toString());
        }
    }
}

void RemoteControl::checkOnline() {
    offlineStatus = true;
    deviceOffline(true);
}

void RemoteControl::clearInformationPanel() {
    QRegularExpressionMatch rx;
    updateDisplayInfo(rx);
    updateDeviceInfo();
}

void RemoteControl::commConnected() {
    ui->connectButton->setChecked(true);
    ui->connectButton->setEnabled(true);

    if(RCSettings::isSaveLatestDeviceEnabled()) {
        QSettings sets(qApp->organizationName(), qApp->applicationName());
        sets.beginGroup("global");
        sets.beginGroup("devices");
        sets.setValue("latestDevice", deviceName);
        sets.endGroup();
        sets.endGroup();
    }
    ui->powerButton->setEnabled(true);
    deviceOnline = true;
    enableControls(true);
    ui->connectButton->setIcon(connectButtonOnIcon);
    clearInformationPanel();
    checkOnline();
}

void RemoteControl::commDisconnected() {
    ui->connectButton->setEnabled(true);
    ui->connectButton->setChecked(false);
    enableControls(false);
    ui->powerButton->setEnabled(false);
    ui->connectButton->setIcon(connectButtonOffIcon);
    deviceOnline = false;
}

void RemoteControl::commError(QString/* socketError*/) {
    commDisconnected();
}

bool RemoteControl::sendCmd(const QString& cmd) {
    return deviceInterface.sendCmd(cmd);
}

void RemoteControl::removeEmpty()
{
    QList<QWidget *> allPanels =
            ui->centralWidget->findChildren<QWidget*>(
                QRegularExpression(".*_internal"), Qt::FindChildrenRecursively);
    foreach(QWidget *wd, allPanels) {
        bool visible = false;
        QList<QPushButton *> btns = wd->findChildren<QPushButton *>(
                    QRegularExpression("rc_btn.*"), Qt::FindChildrenRecursively);
        foreach(QPushButton *but, btns) {
            QString btnName = but->objectName().replace("rc_btn_", "");
            if(!deviceInterface.deviceSettings.value(btnName).isNull()) {
                visible = true;
                break;
            }
        }
        wd->parentWidget()->setVisible(visible);
    }
}

void RemoteControl::changeSettings() {
    clearInformationPanel();
    QList<QPushButton *> allPButtons =
            ui->centralWidget->findChildren<QPushButton*>(
                QRegularExpression("rc_btn.*"), Qt::FindChildrenRecursively);
    //            this->findChildren<QPushButton *>();
    QVariant tiles = deviceInterface.deviceSettings.value("tiles");
    // Merge with default
    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("rc_btn_")) {
            signalMapper->removeMappings(button);
            disconnect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
            QString btnName = button->objectName().replace("rc_btn_", "");
            if(!deviceInterface.deviceSettings.value(btnName).isNull()) {
                signalMapper->setMapping(button, deviceInterface.deviceSettings.value(btnName).toString());
                connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
                if(!tiles.toMap().value(btnName).toString().isEmpty()) {
                    button->setToolTip(tiles.toMap().value(btnName).toString());
                }
            }
            else {
                button->setToolTip("");
            }
        }
    }
    removeEmpty();
}


void RemoteControl::powerClicked() {
    if (offlineStatus ) {
        sendCmd(deviceInterface.deviceSettings.value("powerOn").toString());
    } else {
        sendCmd(deviceInterface.deviceSettings.value("powerOff").toString());
    }
}

void RemoteControl::reconnect() {
    ui->statusDisplayWidget->item(1)->setText(deviceFamily);
    ui->statusDisplayWidget->item(2)->setText(deviceName);
    ui->statusDisplayWidget->item(3)->setText(
                QString(deviceIpAddress).
                append(deviceIpAddress.isEmpty() ? "": QString().asprintf(":%d", deviceIpPort)));

    if(deviceIpPort != 0) {
        settings = RCSettings::load(deviceFamily);
        QVariant var = settings.toMap().value("panel");
        QList<QWidget*>old = ui->centralWidget->findChildren<QWidget*>(
                    QRegularExpression("Panel_"), Qt::FindChildrenRecursively);
        foreach(QWidget* rem, old) {
            delete rem;
        }
        if(var.isValid()) {
            QJsonArray array =  var.toJsonArray();
            int idx = 0;
            foreach (const QJsonValue & value, array) {
                addPanel(idx++, value.toArray());

            }
        }

        QMap<QString, QString> lbls = defaultLabels;
        lbls.detach();

        QMap<QString, QVariant> lbs = settings.toMap().value("labels").toMap();
        foreach (const QString & key, lbs.keys()) {
            lbls.insert(key, lbs.value(key).toString());
        }

        foreach (const QString & key, lbls.keys()) {
            QList<QLabel*> labels = ui->centralWidget->findChildren<QLabel*>(
                        QRegularExpression(QString("rc_lbl_").append(key)), Qt::FindChildrenRecursively);
            foreach(QLabel* label, labels) {
                label->setText(lbls.value(key));
            }
        }
    }
    deviceInterface.reloadDeviceSettings(settings.toMap());
    deviceInterface.connectToDevice(deviceIpAddress, deviceIpPort);
    emit deviceActivated();
}

void RemoteControl::reloadAndReconnect() {
    QAction *action = qobject_cast<QAction *> (sender());
    if (action) {
        reloadAndReconnect(QString(action->text()).replace("/","@@"));
    }
}

void RemoteControl::reloadAndReconnect(QString device) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("devices");

    sets.beginGroup(device);

    deviceFamily = sets.value("deviceFamily", "").toString();
    deviceName = sets.value("deviceName", "").toString();
    deviceIpPort = sets.value("devicePort", deviceIpPort).toUInt();
    deviceIpAddress = sets.value("deviceIPAddress", deviceIpAddress).toString();

    sets.endGroup();

    sets.endGroup();
    sets.endGroup();
    reconnect();
}

void RemoteControl::reloadLatestDevice() {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("devices");
    QString device = sets.value("latestDevice", "").toString();
    sets.endGroup();
    sets.endGroup();

    if(RCSettings::isSaveLatestDeviceEnabled()) {
        reloadAndReconnect(QString(device).replace("/", "@@"));
    }
}
void RemoteControl::closeEvent(QCloseEvent *event) {
    QSettings settings(qApp->organizationName(),
                       qApp->applicationName());
    settings.beginGroup("global");
    settings.setValue("geometryX", geometry().x());
    settings.setValue("geometryY", geometry().y());
    settings.endGroup();
    deviceInterface.disconnect();
    QMainWindow::closeEvent(event);
}

void RemoteControl::settingsClicked() {
    Settings sets(this);
    sets.exec();
}

void RemoteControl::about() {
    AboutDialog about(this);
    about.setModal(true);
    about.exec();

}

void RemoteControl::debugClicked() {
    if(debugDialog != nullptr) {
        closeDebug();
        return;
    }
    debugDialog = new Debug(&deviceInterface, this) ;

    if(this->screen()->geometry().width() < (x() + width() + debugDialog->width()+6)) {
        debugDialog->move(debugDialog->mapFromGlobal(QPoint(x() - 6 - debugDialog->width(), y())));
    } else {
        debugDialog->move(debugDialog->mapFromGlobal(QPoint(x() + width() + 6, y())));
    }

    ui->debugButton->setChecked(true);
    connect(&deviceInterface, SIGNAL(tx(const QString)), debugDialog, SLOT(write(const QString)));
    connect(&deviceInterface, SIGNAL(rx(const QString)), debugDialog, SLOT(read(const QString)));
    connect(debugDialog, SIGNAL(send(const QString)), &deviceInterface, SLOT(sendCmd(const QString)));
    connect(debugDialog, SIGNAL(finished(int)), this, SLOT(closeDebug()));
    debugDialog->show();
}

void RemoteControl::closeDebug() {
    delete debugDialog;
    debugDialog = nullptr;
    ui->debugButton->setChecked(false);
    return;
}
void RemoteControl::setEnableDevMode(bool mode) {
    ui->debugButton->setVisible(mode);
}

bool RemoteControl::eventFilter(QObject *obj, QEvent *event) {
    static bool mouseDown = false;
    static int xRealPos = 0;
    static int yRealPos = 0;

    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if (obj->objectName() == ui->centralWidget->objectName() && event->type() == QEvent::MouseButtonPress) {
        mouseDown = true;
        xRealPos = mouseEvent->globalPosition().x() - x();
        yRealPos = mouseEvent->globalPosition().y() - y();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mouseDown = false;
    } else if (event->type() == QEvent::MouseMove) {
        if (mouseDown) {
            int xPos = x() - (x() - mouseEvent->globalPosition().x()) - xRealPos;
            int yPos = y() - (y() - mouseEvent->globalPosition().y()) - yRealPos;
            this->move(xPos, yPos);
        }
    }
    return false;
}

