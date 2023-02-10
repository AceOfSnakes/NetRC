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
                                              bool load, QString& label, QComboBox& combo, QString& prefferedName) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("history");
    sets.beginGroup(alias);
    QStringList history = sets.value("recent").toStringList();
    QString filename = sets.value("latestFile").toString();
    if(!prefferedName.isEmpty()) {
        QFileInfo info(filename);
        filename = info.absoluteDir().absoluteFilePath(prefferedName);
    }
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
    return resolveFileName(parent, header, filters, alias, true, label, combo, QString().append(""));
}

QString FileDialogWithHistory::resolveLoadFileName(QWidget* parent, QString header, QString &filters, QString alias) {
    QComboBox box;
    return resolveFileName(parent, header, filters, alias, true, QString().append(""), box, QString().append(""));
}

QString FileDialogWithHistory::resolveSaveFileName(QWidget* parent, QString header, QString &filters, QString alias,
                                                  QString& label, QComboBox& combo, QString& prefferedName) {
    return resolveFileName(parent, header, filters, alias, false, label, combo, prefferedName);
}

QString FileDialogWithHistory::resolveSaveFileName(QWidget* parent, QString header, QString &filters, QString alias,
                                                   QString& prefferedName) {
    QComboBox box;
    return resolveFileName(parent, header, filters, alias, false, QString().append(""), box, prefferedName);
}


