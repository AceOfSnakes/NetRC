#ifndef DEVICECONNECTOR_H
#define DEVICECONNECTOR_H

#include <QDialog>
#include <QVariant>
#include <QRegularExpression>

namespace Ui {
class DeviceConnector;
}

class DeviceConnector : public QDialog
{
    Q_OBJECT
private:
    QVariant settings;
public:
    explicit DeviceConnector(QVariant & sets, QWidget *parent = nullptr);
    ~DeviceConnector();
    QString device;
    QString deviceAddress;
    QString deviceFamily;

private slots:
    void on_pushButtonAuto_clicked();
    void on_closeButton_clicked();

    void on_loadConfig_clicked();

    void on_comboBox_activated(const QString &arg1);

private:
    Ui::DeviceConnector *ui;
    void reloadDevicesFamily();
    void select(const QVariant action);
    void SetIpAddress(QString ip1, QString ip2, QString ip3, QString ip4, QString port);
};

#endif // DEVICECONNECTOR_H
