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
    ui(new Ui::RemoteControl)
{

    QHostAddress address;
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint) &(~Qt::WindowMaximizeButtonHint));
    ui->setupUi(this);
    ui->mainWidget->setVisible(true);
    //addPanel(QString("Custom Panel 1"));
    //addPanel(QString("Custom Panel 2"));
   // addPanel(QString("Custom Panel 3"));
   // addPanel(QString("Custom Panel 4"));
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

    m_tray_icon->setContextMenu( tray_icon_menu );
    if(ui->actionMinimize_To_Tray->isChecked()) {
        m_tray_icon->show();
    }
    ui->btn_Connect->setEnabled(true);
    connect(ui->actionMinimize_To_Tray,SIGNAL(changed()), this, SLOT(on_MinimizeToTrayChanged()));
    ui->btn_Connect->setEnabled(true);
    reloadMenu();
    ui->btn_Connect->setEnabled(true);
    show();
    ui->btn_Connect->setEnabled(true);
    redraw();

     //QObject::connect(ui->btn_Connect, SIGNAL(customContextMenuRequested(QPoint)), qApp, SLOT(aboutQt()));
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
    qDebug() <<"changeEvent Sender "<< sender();
    e->accept();
    qDebug() << "changeEvent" << e << e->type()<<e->spontaneous();
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
    QMessageBox::warning(this, qApp->applicationName(),
                                   "just a placeholder when devices were not found :)");
    on_btn_Connect_customContextMenuRequested(QPoint());
}

void RemoteControl::on_btn_Connect_customContextMenuRequested(const QPoint &/*pos*/)
{
    DeviceConnector deviceConnector(settings,this);

    deviceConnector.exec();
    //ui->listWidget->item(1)->setFlags(Hidden(true);
    ui->listWidget->item(1)->setText(deviceConnector.deviceFamily);
    ui->listWidget->item(2)->setText(deviceConnector.device);
}
