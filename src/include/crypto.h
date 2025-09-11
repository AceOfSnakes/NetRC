#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>

class Crypto : public QObject
{
    Q_OBJECT
public:
    explicit Crypto(QObject *parent = nullptr);
    QByteArray decrypt(QByteArray&);
    QByteArray encrypt(QByteArray&);
signals:
};

#endif // CRYPTO_H
