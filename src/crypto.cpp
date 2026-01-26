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
#include "crypto.h"
#include "QDebug"
#include <QRandomGenerator>
#include <QJsonObject>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <QRandomGenerator>

//#define AES_KEY_SIZE 16  // AES-128 uses a 128-bit (16 bytes) key
//#define AES_BLOCK_SIZE 16 // AES block size is 16 bytes

OSSL_PARAM* Crypto::getSSLParams() {
    sslParams.clear();
    sslParams.append(OSSL_PARAM_utf8_string((const char *) OSSL_KDF_PARAM_DIGEST, (void*) SSL_TXT_SHA256, strlen(SSL_TXT_SHA256)));
    sslParams.append(OSSL_PARAM_octet_string((const char *) OSSL_KDF_PARAM_PASSWORD, pass, 8));
    sslParams.append(OSSL_PARAM_octet_string(OSSL_KDF_PARAM_SALT, salt, 16));
    sslParams.append(OSSL_PARAM_int(OSSL_KDF_PARAM_ITER, &iter));
    sslParams.append(OSSL_PARAM_END);

    return sslParams.data();
}

Crypto::Crypto(QVariant *sets, QObject *parent)
    : QObject{parent} {

    qDebug() << *sets;
    auto map = sets->toMap();
    auto value = map.constFind("3-iv.settings.iv.value");
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    EVP_KDF *kdf = EVP_KDF_fetch(NULL, OSSL_KDF_NAME_PBKDF2, NULL);
    EVP_KDF_CTX *kctx = EVP_KDF_CTX_new(kdf);

    if (!EVP_KDF_derive(kctx, key, 16, getSSLParams())) {
        emitOpenSSLErrors();
    }
}

QByteArray Crypto::adaptForCrypto(QByteArray arrayOld)
{
    QByteArray newArray(arrayOld);
    while(newArray.length()% 16 !=0) {
        newArray.append("\2");
    }
    return newArray;
}

void Crypto::emitEncryptedMessage(unsigned char * label, unsigned char * data, int dataLength) {
    QString message = genarateMessage(label, data, dataLength);
    emit encoded(message);
}

void Crypto::emitDecryptedMessage(unsigned char * label, unsigned char * data, int dataLength) {
    QString message = genarateMessage(label, data, dataLength);
    emit decoded(message);
}

void Crypto::emitInfoMessage(unsigned char * label, unsigned char * data, int length) {
    QString message = genarateMessage(label, (unsigned char * ) "", 0);
    emit info(message.append(QByteArray::fromRawData((const char*) data, length).toStdString()));
}

QString Crypto::genarateMessage(unsigned char * label, unsigned char * data, int dataLength) {
    return QString().asprintf("%-8s : ", label).append(
        QByteArray::fromRawData((const char*) data, dataLength).toHex(' ').toUpper());
}

void Crypto::dispalayKeyVars(Direction direction) {
    //emitInfoMessage((unsigned char *)"Pass", (unsigned char *)pass, 8);
    switch (direction) {
    case outbound:
        emitEncryptedMessage((unsigned char *) "Pass", (unsigned char *)pass, 8);
        emitEncryptedMessage((unsigned char *) "Salt", (unsigned char *)salt, 16);
        emitEncryptedMessage((unsigned char *) "Key", (unsigned char *)key, 16);
        break;
    case inbound:
        emitDecryptedMessage((unsigned char *) "Pass", (unsigned char *)pass, 8);
        emitDecryptedMessage((unsigned char *) "Salt", (unsigned char *)salt, 16);
        emitDecryptedMessage((unsigned char *) "Key", (unsigned char *)key, 16);
        break;
    default:
    break;
    }
}

QByteArray Crypto::encrypt(QByteArray arrayOld) {

    EVP_CIPHER_CTX *ctx;

    QByteArray newArray = adaptForCrypto(arrayOld);

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        emitOpenSSLErrors();  return QByteArray();
    }

    //unsigned char ivx[16];

    QByteArray iv(16, Qt::Uninitialized);
    QRandomGenerator::global()->generate(
        reinterpret_cast<quint32*>(iv.data()),
        reinterpret_cast<quint32*>(iv.data()) + 4
        );

    emitEncryptedMessage((unsigned char*) "Msg", (unsigned char*) arrayOld.data(), arrayOld.length());
    emitEncryptedMessage((unsigned char*) "Dest msg", (unsigned char*) newArray.data(), newArray.length());

    //emitInfoMessage((unsigned char*) "Pass", (unsigned char*) pass, 8);
    QByteArray cryptoIv = encryptIV(QByteArray::fromRawData((const char*)iv.constData(),16));
    emitEncryptedMessage((unsigned char*) "IV", (unsigned char*)cryptoIv.data());
    emitEncryptedMessage((unsigned char*) "Key", key);
    int outLen1 = 0;
    int outLen2 = 0;

    QByteArray out;
    out.resize(newArray.size() + 16);

    if (!EVP_EncryptInit_ex(
            ctx, EVP_aes_128_cbc(), nullptr,
            reinterpret_cast<const unsigned char *>(key),
            reinterpret_cast<const unsigned char *>(iv.data())) ||
        !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL) ||
        !EVP_EncryptUpdate(
            ctx, reinterpret_cast<unsigned char *>(out.data()), &outLen1,
            reinterpret_cast<const unsigned char *>(newArray.constData()),
            newArray.size()) ||
        !EVP_EncryptFinal_ex(
            ctx, reinterpret_cast<unsigned char *>(out.data()) + outLen1,
            &outLen2)) {
        emitOpenSSLErrors();
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    out.resize(outLen1 + outLen2);
    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return QByteArray(cryptoIv).append(out);
    //return QByteArray(cryptoIv).append(reinterpret_cast<char *>(out.data()) + outLen1, outLen2);
}

QByteArray Crypto::decrypt(QByteArray array) {

    EVP_CIPHER_CTX *ctx;
    unsigned char ivx[16];
    unsigned char data[32];
    unsigned char ciphertext[128];
    int len = array.size() - 16;
    int outLen1 = 0;
    int outLen2 = 0;
    if(array.size() < 16) {
       return "";
    }

    for (int i = 0; i < array.size(); i++) {
        if (i < 16) {
            ivx[i] = array.at(i);
        } else {
            data[i - 16] = array.at(i);
        }
    }

    //    emitInfoMessage((unsigned char*) "Pass", (unsigned char*) pass, 8);

    QByteArray decIv =
        decryptIV(QByteArray::fromRawData((const char *)ivx, 16));

    if (!(ctx = EVP_CIPHER_CTX_new()) ||

        !EVP_DecryptInit(ctx, EVP_aes_128_cbc(), key,
                         (unsigned char *)decIv.constData()) ||
        !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL) ||
        !EVP_DecryptUpdate(ctx, ciphertext, &outLen1, data, len) ||
        !EVP_CIPHER_CTX_set_padding(ctx, false) ||
        !EVP_DecryptFinal_ex(ctx, ciphertext + outLen1, &outLen2)) {

        emitOpenSSLErrors();
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    emitDecryptedMessage((unsigned char*) "IV", (unsigned char*) decIv.constData());
    emitDecryptedMessage((unsigned char*) "Key", (unsigned char*) key);
    emitDecryptedMessage((unsigned char*) "Data", (unsigned char*) data, len);
    emitDecryptedMessage((unsigned char*) "Data dec", (unsigned char*) ciphertext, outLen1+outLen2);

    return QByteArray(reinterpret_cast<const char *>(ciphertext), outLen1 + outLen2);
}

QByteArray Crypto::encryptIV(QByteArray iv) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    emitEncryptedMessage((unsigned char*) "IV 4 enc", (unsigned char *)iv.constData(), iv.size());
    std::vector<unsigned char> out(16);
    int outLen = 0;

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr)) {
        emitOpenSSLErrors();
        return QByteArray();
    }
    if (!EVP_CIPHER_CTX_set_padding(ctx, 0)) {
        emitOpenSSLErrors();
        return QByteArray();
    }

    if (!EVP_EncryptUpdate(ctx, out.data(), &outLen,
                           (unsigned char *)iv.constData(), iv.size())) {
        emitOpenSSLErrors();
        return QByteArray();
    }

    EVP_CIPHER_CTX_free(ctx);

    return QByteArray(reinterpret_cast<const char *>(out.data()), 16);
}

QByteArray Crypto::decryptIV(QByteArray iv) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    emitDecryptedMessage((unsigned char*) "IV 4 dec", (unsigned char *)iv.constData(), iv.size());
    std::vector<unsigned char> out(16);
    int outLen = 0;

    if(! EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr)) {
        emitOpenSSLErrors();  return QByteArray();
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (!EVP_DecryptUpdate(ctx, out.data(), &outLen, (unsigned char *)iv.constData(), iv.size())) {
        emitOpenSSLErrors();  return QByteArray();
    }

    EVP_CIPHER_CTX_free(ctx);

    return QByteArray(reinterpret_cast<const char *>(out.data()), 16);
}

void Crypto::emitOpenSSLErrors() {
    QString errors;

    ERR_print_errors_cb(
        [](const char* str, size_t len, void* u) -> int {
            QString* out = static_cast<QString*>(u);
            out->append(QString::fromUtf8(str, int(len)));
            return 1;
        },
        &errors
        );

    if (!errors.isEmpty())
        emit info(errors.trimmed());
}
