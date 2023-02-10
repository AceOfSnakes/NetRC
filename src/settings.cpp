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
#include "settings.h"
#include "QDebug"
#include "QFile"
#include "QSettings"
#include <QStyleFactory>
#include "ui_settings.h"
#include "filedialogwithhistory.h"
#include "global.h"
#include "remotecontrol.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    title = windowTitle();
    ui->setupUi(this);
    setWindowTitle(qApp->applicationName().append(". Settings"));

    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    ui->minimizeToTrayCheck->setChecked(sets.value("minimizeToTray", true).toBool());
    ui->rememberLatestCheck->setChecked(sets.value("saveLatestDevice", true).toBool());
    ui->developmentCheck->setChecked(sets.value("devMode", false).toBool());

    sets.endGroup();
    sets.endGroup();

    //    connect(ui->checkBox_minimizeToTray, SIGNAL(stateChanged(int)), parent, SLOT(showHide()));

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(end()));
    connect(ui->loadThemeButton, SIGNAL(clicked()), this, SLOT(loadTheme()));
    connect(ui->defaultThemeButton, SIGNAL(clicked()), this, SLOT(loadDefaultTheme()));
    connect(ui->developmentCheck, SIGNAL(stateChanged(int)), this, SLOT(devModeChanged()));

    setWindowFlags(WINDOW_FLAGS);

}

Settings::~Settings() {
    setWindowTitle(title);
    delete ui;
}

void Settings::saveSettings() {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    sets.setValue("minimizeToTray", QVariant(ui->minimizeToTrayCheck->isChecked()));
    sets.setValue("saveLatestDevice", QVariant(ui->rememberLatestCheck->isChecked()));
    sets.setValue("devMode", QVariant(ui->developmentCheck->isChecked()));
    sets.endGroup();
    sets.endGroup();
}

void Settings::end() {
    saveSettings();
    this->close();
}

QByteArray Settings::loadStyleSheet(QString & filename) {
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray style = file.readAll();
    emit themeChanged(style);

    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.setValue("theme", style);
    sets.endGroup();
    return style;
}

void Settings::loadTheme() {
    QString xfile = FileDialogWithHistory().
            resolveLoadFileName(this, tr("Load Theme file"),
                                tr(" Files")
                                .append(" (*.qss);;")
                                .append(tr("All files"))
                                .append(" (* *.*)"), "theme");
    if (!xfile.isEmpty()) {
        loadStyleSheet(xfile);
    }
}

void Settings::loadDefaultTheme() {
    QString styleFile = (":/style.qss");
    loadStyleSheet(styleFile);
}


void Settings::devModeChanged() {
    ((RemoteControl*)parentWidget())->setEnableDevMode(ui->developmentCheck->isChecked()) ;
}


