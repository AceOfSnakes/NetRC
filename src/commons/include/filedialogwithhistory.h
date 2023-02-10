#ifndef FILEDIALOGWITHHISTORY_H
#define FILEDIALOGWITHHISTORY_H

#include <QString>
#include <QWidget>
#include <QComboBox>

class FileDialogWithHistory
{
private:
    QString resolveFileName(QWidget *parent, QString header, QString &filters, QString alias,
                                  bool load, QString &label, QComboBox &combo, QString &prefferedName);
public:
    FileDialogWithHistory();
    QString resolveLoadFileName(QWidget *parent, QString header, QString &filters, QString alias);
    QString resolveLoadFileName(QWidget *parent, QString header, QString &filters, QString alias,
                                QString &label, QComboBox &combo);

    QString resolveSaveFileName(QWidget *parent, QString header, QString &filters, QString alias,
                                QString &prefferedName= QString().append(""));
    QString resolveSaveFileName(QWidget *parent, QString header, QString &filters, QString alias,
                                QString &label, QComboBox &combo, QString &prefferedName = QString().append(""));

};
#endif // FILEDIALOGWITHHISTORY_H
