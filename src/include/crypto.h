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
#ifndef CRYPTO_H
#define CRYPTO_H

#include <QObject>
#include <QDataStream>
#include <QMetaType>
#include <QVariant>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/params.h>


class Crypto : public QObject
{
    Q_OBJECT

public:
    enum PasswordType { CUSTOM_PASSWORD, VALUE };
    Q_ENUM(PasswordType);
    struct KeySettings {
        PasswordType passwordType = CUSTOM_PASSWORD;
        QString type = "";
        QByteArray password = QByteArray("********", 8);
        int iterations = 0;
        int bitsSize = 0;

        QByteArray salt =
            QByteArray("****************", 8);
    };

    enum IVType { RANDOM_IV, VALUE_IV };
    Q_ENUM(IVType);
    struct IvSettings {
        IVType type = RANDOM_IV;
        int bitsSize = 0;
        QString cipher = "";

        QByteArray value =
            QByteArray("****************", 16);
    };

    struct CryptoSettings {
        QString cipher = "";
        QString padding = "";

        KeySettings key;
        IvSettings iv;
    };
    CryptoSettings cryptoSettings;
    QList<OSSL_PARAM> sslParams;
    unsigned char key[16];

    enum Direction { inbound, outbound, information };

    explicit Crypto(CryptoSettings cryptoSettings, QVariant *sets, QObject *parent = nullptr);
    void updateCryptoSettings(CryptoSettings cryptoSettings);
    QByteArray decrypt(QByteArray);
    QByteArray encrypt(QByteArray);
    QByteArray encryptIV(QByteArray iv);
    QByteArray decryptIV(QByteArray iv);
    OSSL_PARAM *getSSLParams();
public:
    QByteArray adaptForCrypto(QByteArray arrayOld);
    
    void emitEncryptedMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    void emitDecryptedMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    void emitInfoMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    QString genarateMessage(unsigned char * label, unsigned char * data, int dataLength);
    void dispalayKeyVars(Direction direction = Direction::information);
    void emitOpenSSLErrors();

    static QJsonObject convertCryptoToJson(const CryptoSettings& person);
    static QJsonObject convertIvToJson(const IvSettings& person);
    static QJsonObject convertKeyToJson(const KeySettings& person);

    static CryptoSettings parseCryptoData(const QJsonObject& data);
    static IvSettings parseIvData(const QJsonObject& data);
    static KeySettings parseKeyData(const QJsonObject& data);

signals:
    void encoded(const QString &str);
    void decoded(const QString &str);
    void info(const QString &str);
    };

Q_DECLARE_METATYPE(Crypto::CryptoSettings);
Q_DECLARE_METATYPE(Crypto::IvSettings);
Q_DECLARE_METATYPE(Crypto::KeySettings);

#ifndef QT_NO_DEBUG_OUTPUT
QDebug operator<<(QDebug dbg, const Crypto::IvSettings &settings);
QDebug operator<<(QDebug dbg, const Crypto::KeySettings &settings);
QDebug operator<<(QDebug dbg, const Crypto::CryptoSettings &settings);
#endif
#endif // CRYPTO_H
