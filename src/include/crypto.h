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

    int iter = 16384;

public:
    // QString pass;
    QList<OSSL_PARAM> sslParams;

    //unsigned char pass[8] = {'1', '2', '3', '4', '5', '6', '7', '8'} ;
    unsigned char pass[8] = {'P', '7', 'R', 'N', 'F', 'K', '6', '6'};
    unsigned char key[16];
    // unsigned char salt[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
    //                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    enum Direction { inbound, outbound ,information};

    unsigned char salt[16] = {0x63, 0x61, 0xb8, 0x0e, 0x9b, 0xdc, 0xa6, 0x63,
                            0x8d, 0x07, 0x20, 0xf2, 0xcc, 0x56, 0x8f, 0xb9};

    explicit Crypto(QVariant *sets, QObject *parent = nullptr);
    QByteArray decryptIV(QByteArray);
    QByteArray decrypt(QByteArray);
    QByteArray encrypt(QByteArray);
    QByteArray decodeIV(unsigned char iv[]);
    QByteArray encryptIV(unsigned char iv[]);
    OSSL_PARAM* getSSLParams();
public:
    QByteArray adaptForCrypto(QByteArray arrayOld);
    
    void emitEncryptedMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    void emitDecryptedMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    void emitInfoMessage(unsigned char * label, unsigned char * data, int dataLength = 16);
    QString genarateMessage(unsigned char * label, unsigned char * data, int dataLength);

    void dispalayKeyVars(Direction direction = Direction::information);

signals:
    void encoded(const QString &str);
    void decoded(const QString &str);
    void info(const QString &str);

    };

#endif // CRYPTO_H
