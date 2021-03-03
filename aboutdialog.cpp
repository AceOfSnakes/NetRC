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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "global.h"
#include "commons.h"
#include <QImage>
#include <QtGui>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    setWindowFlags(WINDOW_FLAGS);
    setFixedSize(width(), height());
    ui->label->setText(qApp->applicationName() + ". Version: " + qApp->applicationVersion());
    ui->label_os->setText(
                QString("OS: ")
                .append(QSysInfo::prettyProductName())
                .append(" / Product version: ")
                .append(QSysInfo::kernelType())
                .append(" ")
                .append(QSysInfo::productVersion()
                .append(" / Kernel version: ")
                .append(QSysInfo::kernelVersion())
                ));
    ui->compiler->setText(Commons::compilerQString());
    ui->buttonBox->clear();
    setWindowTitle("About " + qApp->applicationName());
    ui->buttonBox->addButton(ui->buttonBox->Ok);
    connect(ui->qt, SIGNAL(customContextMenuRequested(QPoint)), qApp, SLOT(aboutQt()));
    ui->buttonBox->connect(ui->buttonBox->button(ui->buttonBox->Ok), SIGNAL(clicked(bool)), this,
                           SLOT(close()));
}

AboutDialog::~AboutDialog() {
    delete ui;
}



