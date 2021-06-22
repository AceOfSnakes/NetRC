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
#include "filedialogwithhistory.h"
#include <QFileDialog>
#include <QSettings>
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QString>

FileDialogWithHistory::FileDialogWithHistory() {

}

QString FileDialogWithHistory::resolveFileName(QWidget* parent, QString header, QString &filters, QString alias,
                                              bool load, QString& label, QComboBox& combo) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("history");
    sets.beginGroup(alias);
    QStringList history = sets.value("recent").toStringList();
    QString filename = sets.value("latestFile").toString();
    sets.endGroup();
    sets.endGroup();

    QFileDialog fd(parent, header, filename, filters);

    if (load) {
        fd.setOptions(QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    } else {
        fd.setOptions(QFileDialog::DontUseNativeDialog);
        fd.setAcceptMode(QFileDialog::AcceptSave);
    }
    fd.setHistory(history);

    QLayout* layout = fd.layout();

    // TODO WTF - original combo usage crash the program
    QComboBox* box = new QComboBox();

    QGridLayout* gridbox = qobject_cast<QGridLayout*>(layout);
    if (gridbox && combo.count() > 0) {
//        gridbox->setStyleSheet("padding-top: 3px");
        for(int cnt = 0; cnt< combo.count(); cnt++) {
            box->addItem(combo.itemText(cnt));
        }
        gridbox->addWidget(new QLabel(label));
        gridbox->addWidget(box);
    }

    fd.setLayout(gridbox);

    if (!fd.exec()) {
        return QString();
    }
    if (gridbox && combo.count() > 0) {
        combo.setCurrentIndex(box->currentIndex());
    }
    filename = fd.selectedFiles().at(0);

    QFileInfo file(filename);
    if (file.exists() || ! load) {
        sets.beginGroup("history");
        sets.beginGroup(alias);
        history.removeDuplicates();
        history.removeAll(file.absolutePath());
        history.append(file.absolutePath());
        sets.setValue("recent", history);
        sets.setValue("latestFile", filename);
        sets.endGroup();
        sets.endGroup();

    }

    return filename;
}

QString FileDialogWithHistory::resolveLoadFileName(QWidget* parent, QString header, QString &filters, QString alias,
                                                  QString& label, QComboBox& combo) {
    return resolveFileName(parent, header, filters, alias, true, label, combo);
}

QString FileDialogWithHistory::resolveLoadFileName(QWidget* parent, QString header, QString &filters, QString alias) {
    QComboBox box;
    return resolveFileName(parent, header, filters, alias, true, QString().append(""), box);
}

QString FileDialogWithHistory::resolveSaveFileName(QWidget* parent, QString header, QString &filters, QString alias,
                                                  QString& label, QComboBox& combo) {
    return resolveFileName(parent, header, filters, alias, false, label, combo);
}

QString FileDialogWithHistory::resolveSaveFileName(QWidget* parent, QString header, QString &filters, QString alias) {
    QComboBox box;
    return resolveFileName(parent, header, filters, alias, true, QString().append(""), box);
}

