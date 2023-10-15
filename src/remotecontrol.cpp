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
#include <QNetworkProxy>
#include <QSpacerItem>
#include <QStyleFactory>
#include <QResource>
#include <QLatin1String>
#include <QWindowStateChangeEvent>
#include <QMenu>
#include <QFileDialog>
#include <QStyleHints>
#include <QRect>
#include <QSettings>
#include <QJsonDocument>
#include <QMessageBox>
#include <QJsonObject>
#include <QSpacerItem>
#include <QJsonArray>
#include <QNetworkAddressEntry>
#include <QFontDatabase>
#include <QWidget>
#include "aboutdialog.h"
#include "debug.h"
#include "commons.h"
#include "deviceconnector.h"
#include "rcsettings.h"
#include "settings.h"

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
    foreach(QPushButton *action, ui->centralWidget->findChildren<QPushButton*>(
                                      Qt::FindChildrenRecursively)) {
        originalIcons.insert(action->objectName(), action->icon());
    }

    this->setWindowIcon(QIcon(QString(":/images/").append(qApp->applicationName()).append(".png")));
    restoreSettings();
    ui->mainWidget->setVisible(true);
    loadSettings();

    QFont font = ui->statusDisplayWidget->font();
    font.setFamily("Courier New");
    font.setBold(true);
    font.setStyleHint(QFont::Monospace);
    ui->statusDisplayWidget->setFont(font);

    QNetworkProxy proxy;
    QNetworkProxy::setApplicationProxy(proxy);

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
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &RemoteControl::colorSchemeChanged);
}
void RemoteControl::colorSchemeChanged(Qt::ColorScheme scheme) {
    qDebug()<< scheme;
}

QIcon RemoteControl::invertedIcon(QIcon icon) {
    QIcon invertedIcon;
    for(auto singleMode : {QIcon::Normal, QIcon::Disabled, QIcon::Active, QIcon::Selected}) {
        for(auto singleState : {QIcon::On, QIcon::Off}) {
            const auto avalSize = icon.availableSizes(singleMode, singleState );
            for(auto& singleSize : avalSize){
                QImage tempImage = icon.pixmap(singleSize, singleMode, singleState).toImage();
                tempImage.invertPixels();
                invertedIcon.addPixmap(QPixmap::fromImage(std::move(tempImage)), singleMode, singleState);
            }
        }
    }
    return invertedIcon;
}

void RemoteControl::restoreSettings() {

    QSettings settings(qApp->organizationName(),
                       qApp->applicationName());

    settings.beginGroup("global");
    if(!settings.value("theme").isValid()) {
        settings.setValue("theme", QResource(":/commons/style/black.qss").uncompressedData().constData());
    }

    setGeometry(settings.value("geometryX").toInt(),settings.value("geometryY").toInt(),0,0);
    QByteArray style = settings.value("theme").toByteArray();
    changeTheme(style);
    settings.endGroup();
}

void RemoteControl::repaintDebugDialog() {
    if(debugDialog != nullptr) {
        foreach(QString key, debugDialog->originalIcons.keys() ) {
            qDebug() << "restore" << key << "repaintDebugDialog";
            debugDialog->findChild<QPushButton*>(key, Qt::FindChildrenRecursively)
                ->setIcon(debugDialog->originalIcons.value(key));
        }
        if(this->styleSheet().isEmpty()) {
            const QPalette defaultPalette;

            if (defaultPalette.color(QPalette::WindowText).lightness()
                > defaultPalette.color(QPalette::Window).lightness()) {
                foreach(QPushButton *action, debugDialog->findChildren<QPushButton*>(
                                                  Qt::FindChildrenRecursively)) {
                    qDebug() << action->objectName() << "repaintDebugDialog";
                    action->setIcon(invertedIcon(debugDialog->originalIcons.value(action->objectName())));
                }
            }
        }
    }
}

void RemoteControl::changeTheme(QByteArray style) {
    foreach(QString key, originalIcons.keys() ) {
        ui->centralWidget->findChild<QPushButton*>(key, Qt::FindChildrenRecursively)
            ->setIcon(originalIcons.value(key));
    }
    // Media control icons
    if(style.isEmpty()) {
        ui->rc_btn_colorYellow->setStyleSheet("background-color : yellow");
        ui->rc_btn_colorGreen->setStyleSheet("background-color : green");
        ui->rc_btn_colorRed->setStyleSheet("background-color : red");
        ui->rc_btn_colorBlue->setStyleSheet("background-color : blue");
        setAttribute(Qt::WA_NoSystemBackground, false);
        const QPalette defaultPalette;

        if (defaultPalette.color(QPalette::WindowText).lightness()
            > defaultPalette.color(QPalette::Window).lightness()) {
            foreach(QPushButton *action, ui->centralWidget->findChildren<QPushButton*>(
                                              QRegularExpression("btn.*"),
                                              Qt::FindChildrenRecursively)) {
                action->setIcon(invertedIcon(originalIcons.value(action->objectName())));
            }
            ui->settingsButton->setIcon(invertedIcon(originalIcons.value("settingsButton")));
            ui->aboutButton->setIcon(invertedIcon(originalIcons.value("aboutButton")));
            ui->debugButton->setIcon(invertedIcon(originalIcons.value("debugButton")));
            ui->exitButton->setIcon(invertedIcon(originalIcons.value("exitButton")));
            ui->minButton->setIcon(invertedIcon(originalIcons.value("minButton")));
            //
        }

    } else {
        ui->rc_btn_colorYellow->setStyleSheet("");
        ui->rc_btn_colorGreen->setStyleSheet("");
        ui->rc_btn_colorRed->setStyleSheet("");
        ui->rc_btn_colorBlue->setStyleSheet("");
        setAttribute(Qt::WA_NoSystemBackground);
    }
    QString styleSheet = QLatin1String(style);
    this->setStyleSheet(styleSheet);
    repaintDebugDialog();
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

    connect((&deviceInterface), SIGNAL(specialControl(QString, bool)),
            this,  SLOT(specialControl(QString, bool)));

    connect((&deviceInterface), SIGNAL(updateDeviceInfo()), this,  SLOT(updateDeviceInfo()));

    // Signal mapper
    connect(signalMapper, SIGNAL(mappedString(const QString&)), this, SLOT(sendCmd(QString)));

    // Timer for checking online device status
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkOnlineInternal()));
    timer->start(1000);

    // Buttons
    connect(ui->debugButton, SIGNAL(clicked()), this, SLOT(debugClicked()));
    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(quit()));
    connect(ui->minButton, SIGNAL(clicked()), this, SLOT(showHide()));
    connect(ui->aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui->connectButton, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(connectCustomMenuRequested(QPoint)));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(settingsClicked()));
    connect(ui->powerButton, SIGNAL(clicked()), this, SLOT(powerClicked()));



}

void RemoteControl::initTray() {
    trayGray = QIcon(QString(":/images/tray/NetRC-Gray.png"));
    trayGreen = QIcon(QString(":/images/tray/NetRC-Green.png"));
    trayRed = QIcon(QString(":/images/tray/NetRC-Red.png"));
    trayIcon = new QSystemTrayIcon(trayGray, this);
    trayIcon->setToolTip(qApp->applicationName());

    connect(trayIcon, &QSystemTrayIcon::activated, this, &RemoteControl::showHideWithReason);

    QAction *quitAction = new QAction(QIcon(QString(":/images/nav/special/close.png")), "Exit", trayIcon );
    connect( quitAction, SIGNAL(triggered()), this, SLOT(quit()) );

    QAction *hideAction = new QAction(QIcon(QString(":/images/nav/special/showHide.png")), "Show / Hide", trayIcon );
    connect( hideAction, SIGNAL(triggered()), this, SLOT(showHide()) );

    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->setStyleSheet(this->styleSheet());
    trayIconMenu->addAction( hideAction );
    trayIconMenu->addAction( quitAction );

    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(quit()));
    trayIcon->setContextMenu(trayIconMenu);
    if (RCSettings::isMinimizeToTrayEnabled() ) {
        trayIcon->show();
    }
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
    QWidget *widInt = new QWidget(ui->centralWidget);
    widInt->setEnabled(true);
    widInt->setVisible(true);
    QString panelName = QString().asprintf("Panel_%d", panelIdx);
    widInt->setObjectName(panelName);
    QGridLayout* layout = new QGridLayout(widInt);
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
    QString group = sets.value("group").toString();
    uint panelPosition = sets.value("position").toUInt();
    QList<QPushButton *> list;
    if(columns > 0 || panelPosition > 0) {
        buttons_count = columns;
        // skip first button as settings element
        offset = 1;
    }
    uint rows = buttons.size() + buttons_count - 1 - offset;
    rows = buttons_count > 0 ? rows / buttons_count: rows;
    //const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    for (row = 0; row < rows; row++) {
        for (column = 0; column < buttons_count; column++) {
            QMap<QString, QVariant>button = buttons.at(
                        offset + (row * buttons_count) + column).toVariant().toMap();
            QString title = button.value("title").toString();
            QString btnGroup = button.value("group").toString();
            if(!title.isEmpty()) {
                QPushButton *btn = new QPushButton(title);
                //btn->setFont(fixedFont);
                btn->setFont(QFont(btn->font().family(), 7, QFont::Bold));
                btn->setObjectName(QString().asprintf("rc_btn_").append(button.value("btn").toString()));
                btn->setMinimumWidth(50);
                btn->setMinimumHeight(28);
                btn->setMaximumHeight(28);
                btn->setEnabled(false);
                layout->addWidget(btn, row, column);
                if(btnGroup.isEmpty() || group == btnGroup) {
                    list.append(btn);
                }
            }
        }
    }
    if(!group.isEmpty()) {
        buttonGroups.insert(group, list);
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

void RemoteControl::showAll( ) {
    qDebug() << "showAll !!!";
    show();
    foreach(QWidget *child, this->findChildren<QDialog*>(Qt::FindChildrenRecursively)) {
        qDebug() << "showAll child" << child;
        child->showNormal();
    }
}

void RemoteControl::showMinimizedAll( ) {
    qDebug() << "showMinimizedAll !!!";
    showMinimized();
    foreach(QWidget *child, this->findChildren<QDialog*>(Qt::FindChildrenRecursively)) {
        qDebug() << "showMinimizedAll child  " << child;
        child->showMinimized();
    }
}

void RemoteControl::hideAll( ) {
    qDebug() << "hideAll !!!";
    hide();
    foreach(QWidget *child, this->findChildren<QDialog*>(Qt::FindChildrenRecursively)) {
        qDebug() << "hideAll child" << child;
        child->showMinimized();
    }
}

void RemoteControl::showHide( ) {
    if (RCSettings::isMinimizeToTrayEnabled() ) {
        isVisible() ? hideAll() : showAll();
        activateWindow();
    }
    else {
        isVisible() ? showMinimizedAll()  : showAll();
        activateWindow();
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
    switch (e->type()) {
    case QEvent::WindowStateChange:
    {
        QWindowStateChangeEvent* event = static_cast < QWindowStateChangeEvent* >( e );
        if(!isMinimized() && isVisible()) {
            show();
            break;
        }
        if(((event->oldState() == Qt::WindowNoState ) && isMinimized()) ||
                ((event->oldState() &Qt::WindowMinimized ) && !isMinimized() )) {
            showHide();
        }
    }
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

    if(this->styleSheet().isEmpty()) {
        const QPalette defaultPalette;

        if (defaultPalette.color(QPalette::WindowText).lightness()
            > defaultPalette.color(QPalette::Window).lightness()) {
            foreach(QPushButton *action, deviceConnector.findChildren<QPushButton*>(
                                              Qt::FindChildrenRecursively)) {
                qDebug() << action->objectName() << "repaintDebugDialog";
                action->setIcon(invertedIcon(action->icon()));
            }
        }
    }



    deviceConnector.exec();

    if(!deviceConnector.deviceFamily.isEmpty()) {
        deviceIpAddress = deviceConnector.deviceIPAddress;
        qDebug() << "deviceIpAddress" << deviceIpAddress;
        if(!deviceIpAddress.isEmpty()) {
            deviceName = deviceConnector.device;
            settings.swap(deviceConnector.settings);
            deviceFamily = deviceConnector.deviceFamily;
            deviceIpPort = deviceConnector.devicePort;
            reconnect();
            checkOnline();
        }
    }

}
void RemoteControl::connectCustomMenuRequested(const QPoint &pos) {
    QList<QString> devices = RCSettings::devicesList();

    if(devices.isEmpty()) {
        newDevice();
    }
    else {
        QMenu menu(ui->connectButton);
        foreach(QString device, devices) {
            menu.addAction(QString(device), this, SLOT(reloadAndReconnect()));
        }
        menu.addSeparator();
        menu.addAction("&Device Management", this, SLOT(newDevice()));
        menu.exec(this->mapToGlobal(pos));
    }
}

void RemoteControl::connectDevice() {
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
    QIcon newIcon = (offline) ? trayRed : trayGreen;
    if(trayIcon->icon().data_ptr() != newIcon.data_ptr()) {
        trayIcon->setIcon(newIcon);
    }
    enableControls(!offline);
    offlineStatus = offline;
}

void RemoteControl::enableControls(bool enable)
{
    QList<QPushButton *> allPButtons =
            ui->centralWidget->findChildren<QPushButton*>(
                QRegularExpression("rc_btn.*"), Qt::FindChildrenRecursively);

    foreach(QPushButton *button, enabledButtons) {
        allPButtons.removeAll(button);
    }

    ui->statusDisplayWidget->setEnabled(enable);

    foreach (QPushButton *button, allPButtons) {
        if(button->objectName().startsWith("rc_btn_")) {
            QString btnName = button->objectName().replace("rc_btn_","");
            if(deviceInterface.deviceSettings.contains(btnName)) {
                button->setEnabled(enable);
            } else {
                button->setEnabled(false);
            }
        }
    };
}

void RemoteControl::onConnect() {
    /*
    if (!deviceInterface.isConnected()) {
        // connect
        connectDevice();
    }
    else {
        // disconnect
        enableControls(false);
        deviceInterface.disconnect();
        deviceOnline = false;
    }*/
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
    if(deviceInterface.isConnected()) {
        QString app = qApp->applicationName().append(". ").append(deviceName);
        this->setWindowTitle(app);
        if (RCSettings::isMinimizeToTrayEnabled()) {
            trayIcon->showMessage(deviceName, "Connected", QIcon(QString(":/images/").append(qApp->applicationName()).append(".png")));
            trayIcon->setToolTip(app);
        }
    }
}

void RemoteControl::commDisconnected() {
    ui->connectButton->setEnabled(true);
    ui->connectButton->setChecked(false);
    enableControls(false);
    ui->powerButton->setEnabled(false);
    ui->connectButton->setIcon(connectButtonOffIcon);
    deviceOnline = false;
    if(trayIcon->icon().data_ptr() != trayGray.data_ptr()) {
        trayIcon->setIcon(trayGray);
    }
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
    enabledButtons.clear();
    //emit deviceActivated();
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
    connect(&sets, SIGNAL(themeChanged(QByteArray)), this, SLOT(changeTheme(QByteArray)));
    connect(&sets, SIGNAL(devModeChanged(bool)), this, SLOT(setEnableDevMode(bool)));
    sets.exec();
    RCSettings::isMinimizeToTrayEnabled() ? trayIcon->show(): trayIcon->hide();
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
    repaintDebugDialog();
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
    Commons::moveWindow(obj, event, this);
    return false;
}

void RemoteControl::specialControl(const QString control, bool enabled) {
    enabledButtons.clear();
    QList<QPushButton*>value = buttonGroups.value(control);
    foreach(QPushButton* btn, value) {
        if(enabled) {
            btn->setEnabled(enabled);
            enabledButtons.insert(btn);
        }
    }
}

void RemoteControl::quit() {
    this->close();
    exit(0);
}



