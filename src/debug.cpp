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
#include <QMetaEnum>
#include "ui_debug.h"

Debug::Debug(DeviceInterface *deviceInterface, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Debug) {

    devInterface = deviceInterface;
    ui->setupUi(this);
    setWindowTitle(qApp->applicationName() + " RX/TX Console");

    connect(ui->displayAllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchAll(int)));
    connect(ui->eraseDebugOutput, SIGNAL(clicked()), this, SLOT(cleanDebugOutput()));
    connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pauseClicked()));
    connect(ui->maxLines, SIGNAL(editingFinished()), this, SLOT(changeMaxLines()));
    connect(ui->commandLine, SIGNAL(editingFinished()), this, SLOT(sendCommand()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendCommand()));
    connect(ui->clearCommandLineButton, SIGNAL(clicked()), this, SLOT(clearCommandLine()));
    connect(ui->coloredOutputCheckBox, SIGNAL(checkStateChanged(Qt::CheckState)),
            this, SLOT(changeAgenda()));

    ui->maxLines->setText("50");
    changeAgenda();

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
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.setValue("debugEnabled", true);
    sets.endGroup();
}

Debug::~Debug() {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.setValue("debugEnabled", false);
    sets.endGroup();

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

void Debug::display(const Color color, const QString str, bool crypted) {
    QString htmlColor;
    QString circ = circle;
    if(ui->coloredOutputCheckBox->isChecked()) {
        ui->textEdit->setTextColor(mapColored.value(color));
    } else {
        circ = mapNonColored.value(color);
        ui->textEdit->setTextColor(QColor( "lightgray" ));
    }
    ui->textEdit->append(QString(circ)
                             .append(crypted? " ⚿ " :" ").append(str));
}

void Debug::read(const QString str, bool crypted) {
    if(isRsNotDisplayed(str)) { return; }
    display(inbound, str, crypted);
}

QString Debug::arrayToString(const QByteArray array) {
    QString ret = QByteArray::fromRawData((const char*)array,
                                          array.length()).toHex(' ').toUpper();
    int correction = 0;
    int originalLength = ret.length();
    for(int i = 16; i < array.length(); i += 16 ) {
        ret.insert(i*3 + correction, "\n     ");
        correction += ret.length() - originalLength;
    }
    return ret;
}

void Debug::writeArray(const QByteArray array, bool crypted) {
    QString rq = arrayToString(array);
    if(isRqNotDisplayed(rq)) { return; }
    display(outbound, rq, crypted);
}

void Debug::readArray(const QByteArray array, bool crypted) {
    QString rs = arrayToString(array);
    if(isRqNotDisplayed(rs)) { return; }
    display(inbound, rs, crypted);
}

void Debug::write(const QString str, bool crypted) {
    if(isRqNotDisplayed(str)) { return; }
    display(outbound, str, crypted);
}

void Debug::error(const QString str, bool crypted) {
    display(alert, str, crypted);
}

void Debug::warn(const QString str, bool crypted) {
    if(pause) return;
    display(information, str, crypted);
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

void Debug::changeAgenda() {
    QMetaEnum colorMetaData = QMetaEnum::fromType<Color>();
    foreach(QLabel *label, this->findChildren<QLabel*>(Qt::FindChildrenRecursively)) {
        QString color = label->objectName().split("ColorLabel").at(0);
        bool ok = false;
        Color m = Color(colorMetaData.keyToValue(color.toUtf8().constData(), &ok));
        if(label->objectName().endsWith("ColorLabel")){
            // Colored
            if(ui->coloredOutputCheckBox->isChecked()) {
                label->setText(label->text().replace(0, 1, circle));
                label->setStyleSheet(QString("QLabel { color : ")
                                         .append(mapColored.value(m).name())
                                         .append("}"));
            // Non Colored
            } else {
                label->setStyleSheet(QString("QLabel { color : lightgray}"));
                label->setText(label->text().replace(0, 1, mapNonColored.value(m)));
            }
        }
    }
}
