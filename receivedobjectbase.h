#ifndef RECEIVEDOBJECTBASE_H
#define RECEIVEDOBJECTBASE_H

#include <QObject>
#include <QStringList>

class ReceivedObjectBase : public QObject
{
    Q_OBJECT
public:
    explicit ReceivedObjectBase(QObject *parent = 0);
    ~ReceivedObjectBase();
    virtual QString getResponseID() = 0;
    virtual QStringList getMsgIDs() = 0;
    bool parseString(const QString& str, const bool is_pioneer);
    virtual bool parseString(QString str) = 0;

protected:
    bool            m_IsPioneer;
    QString DecodeHexString(const QString& hex);

signals:

public slots:
};

#endif // RECEIVEDOBJECTBASE_H
