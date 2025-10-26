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
#include "debug.h"
#include "QCheckBox"
#include "ui_debug.h"

Debug::Debug(DeviceInterface *deviceInterface, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Debug) {

    devInterface = deviceInterface;
    ui->setupUi(this);
    setWindowTitle(qApp->applicationName() + " RX/TX Console");
    //setWindowTitle(qApp->applicationName() + " RX/TX Console " + deviceInterface->deviceName);

    connect(ui->displayAllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchAll(int)));
    connect(ui->eraseDebugOutput, SIGNAL(clicked()), this, SLOT(cleanDebugOutput()));
    connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pauseClicked()));
    connect(ui->maxLines, SIGNAL(editingFinished()), this, SLOT(changeMaxLines()));
    connect(ui->commandLine, SIGNAL(editingFinished()), this, SLOT(sendCommand()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendCommand()));
    connect(ui->clearCommandLineButton, SIGNAL(clicked()), this, SLOT(clearCommandLine()));

    ui->maxLines->setText("50");

    foreach(QPushButton *action, this->findChildren<QPushButton*>(
                                      Qt::FindChildrenRecursively)) {
        originalIcons.insert(action->objectName(),action->icon());
    }
    this->setWindowFlags(windowFlags() &(~Qt::WindowMinMaxButtonsHint));
    //this->setWindowFlags(Qt::WindowNoState);
    changeMaxLines();

    adjustSize();
    setMaximumSize(minimumSize());

    QFont font = ui->textEdit->font();
    font.setFamily("Courier new");

    font.setStyleHint(QFont::Monospace, QFont::ForceOutline);
    ui->textEdit->setFont(font);

}

Debug::~Debug() {
    delete ui;
}

bool Debug::isRqNotDisplayed(const QString str) {
    return pause ||
           ((!ui->rqRegex->text().isEmpty()) ?
                         !str.contains(ui->rqRegex->text()) :
                         (!ui->pingRq->isChecked() &&
                          devInterface->pingCommands.contains(str)));
}

bool Debug::isRsNotDisplayed(const QString str) {
        return pause ||
           ((!ui->rsRegex->text().isEmpty()) ?
                         !str.contains(ui->rsRegex->text()) :
                         (!ui->timeRs->isChecked() && devInterface->isTimeRs(str)) ||
                         (!ui->pingRs->isChecked() && devInterface->isPingRs(str)) ||
                         (!ui->deviceRs->isChecked() && devInterface->isDeviceIdRs(str)));
}

void Debug::read(const QString str) {
    if(isRsNotDisplayed(str)) { return; }
    ui->textEdit->append(QString(" 🟢 ").append(str));
}

void Debug::write(const QString str) {
    if(isRqNotDisplayed(str)) { return; }
    ui->textEdit->append(QString(" 🔵 ").append(str));
}

void Debug::error(const QString str) {
    ui->textEdit->append(QString(" 🔴 ").append(str));
}

void Debug::warn(const QString str) {
    ui->textEdit->append(QString(" 🟡 ").append(str));
}

void Debug::pauseClicked() {
    pause = !pause;
    if(pause) {
        ui->pauseButton->setIcon(QIcon(":/images/nav/play/play.png"));
    } else {
        ui->pauseButton->setIcon(QIcon(":/images/nav/play/pause.png"));
    }
    originalIcons.insert(ui->pauseButton->objectName(), ui->pauseButton->icon());
    emit(iconChanged(*ui->pauseButton));
}

void Debug::cleanDebugOutput() {
    ui->textEdit->document()->clear();
}

void Debug::sendCommand() {
    emit send(ui->commandLine->text());
}

void Debug::clearCommandLine() {
    ui->commandLine->setText("");
}

void Debug::changeMaxLines() {
    ui->textEdit->document()->setMaximumBlockCount(ui->maxLines->text().toInt());
}

void Debug::switchAll(int checkBoxStatus) {
    foreach(QCheckBox* checkBox, ui->groupBox->findChildren<QCheckBox*>(
                QRegularExpression(""),
                Qt::FindChildrenRecursively)) {
        checkBox->setChecked(checkBoxStatus!= 0);
    }
}
