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
#include "commons.h"
#include <QImage>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QtGui>
#include <openssl/crypto.h>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent), contributor(0) , ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    setWindowFlags(WINDOW_FLAGS);
    setFixedSize(width(), height());
    ui->label->setText(qApp->applicationName() + ". Version: " + qApp->applicationVersion()
                       .append(" / ").append(SSLeay_version(SSLEAY_VERSION)));
    ui->label_os->setText(
                QString("OS: ")
                .append(Commons::prettyProductName())
                .append("\nProduct version: ")
                .append(QSysInfo::kernelType())
                .append(" ")
                .append(QSysInfo::productVersion()
                .append("\nKernel version: ")
                .append(QSysInfo::kernelVersion())
                ));
    ui->compiler->setText(Commons::compilerQString());
    ui->buttonBox->clear();
    setWindowTitle("About " + qApp->applicationName());
    ui->buttonBox->addButton(ui->buttonBox->Ok);
    connect(ui->qt, SIGNAL(customContextMenuRequested(QPoint)), qApp, SLOT(aboutQt()));
    ui->buttonBox->connect(ui->buttonBox->button(ui->buttonBox->Ok), SIGNAL(clicked(bool)), this,
                           SLOT(close()));
    QDir info(":/images/contributors/");
    contributors = info.entryInfoList(QDir::Files);
    QDir info1(":/images/contributors/author");
    contributors.append(info1.entryInfoList(QDir::Files));
    //qDebug()<< contributors;
    if(contributors.length() > 0) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
        ui->labelAuthor->setGraphicsEffect(effect);
        QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(2500);
        anim->setStartValue(contributors.length());
        anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        anim->setLoopCount(-1);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
        // connect(anim, &QPropertyAnimation::currentLoopChanged, [=]() {
        //     changeContributor();
        // });
        connect(anim, SIGNAL(currentLoopChanged(int)), this, SLOT(changeContributor()));
    }
}

AboutDialog::~AboutDialog() {
    delete ui;
}

QPixmap GetNew(QImage& img) {
    return QPixmap::fromImage(img);
}

void  AboutDialog::changeContributor() {
    contributor = contributor % contributors.length();
    QImage img;
    QString x = contributors.at(contributor).canonicalFilePath();
    qDebug() << contributor << x;
    img.load(x);
    ui->label_4->setText(contributors.at(contributor).baseName());
    //QPixmap xxx = QPixmap::fromImage(img);
    ui->labelAuthor->setPixmap(GetNew(img));
    contributor++;
}





