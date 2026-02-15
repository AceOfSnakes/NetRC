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
#include <QMetaEnum>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <QRandomGenerator>

OSSL_PARAM* Crypto::getSSLParams() {
    sslParams.clear();
    sslParams.append(OSSL_PARAM_utf8_string((const char *) OSSL_KDF_PARAM_DIGEST,
                                            (void*) SSL_TXT_SHA256, strlen(SSL_TXT_SHA256)));

    sslParams.append(OSSL_PARAM_octet_string((const char *) OSSL_KDF_PARAM_PASSWORD,
                                             (void *) cryptoSettings.key.password.constData(), 8));
    sslParams.append(OSSL_PARAM_octet_string(OSSL_KDF_PARAM_SALT,
                                             (void *) cryptoSettings.key.salt.constData(), 16));
    sslParams.append(OSSL_PARAM_int(OSSL_KDF_PARAM_ITER, &cryptoSettings.key.iterations));
    sslParams.append(OSSL_PARAM_END);

    return sslParams.data();
}

Crypto::Crypto(CryptoSettings cSettings, QVariant *sets, QObject *parent)
    : QObject(parent) {
    updateCryptoSettings(cSettings);
    //qDebug() << *sets;
}

void  Crypto::updateCryptoSettings(CryptoSettings csets) {
    qDebug() << "Crypto::updateCryptoSettings" << csets;
    cryptoSettings = csets;

    //qDebug() << cryptoSettings;

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
        emitEncryptedMessage((unsigned char *) "Pass",
                             (unsigned char *) cryptoSettings.key.password.constData(), 8);
        emitEncryptedMessage((unsigned char *) "Salt",
                             (unsigned char *) cryptoSettings.key.salt.constData(), 16);
        emitEncryptedMessage((unsigned char *) "Key",
                             (unsigned char *) key, 16);
        break;
    case inbound:
        emitDecryptedMessage((unsigned char *) "Pass",
                             (unsigned char *) cryptoSettings.key.password.constData(), 8);
        emitDecryptedMessage((unsigned char *) "Salt",
                             (unsigned char *) cryptoSettings.key.salt.constData(), 16);
        emitDecryptedMessage((unsigned char *) "Key",
                             (unsigned char *) key, 16);
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
    unsigned char data[1024];
    unsigned char ciphertext[1024];
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

QJsonObject Crypto::convertCryptoToJson(const CryptoSettings& data) {
    QJsonObject jsonObject;
    jsonObject["cipher"] = data.cipher;
    jsonObject["padding"] = data.padding;
    jsonObject["key"] = convertKeyToJson(data.key);
    jsonObject["iv"] = convertIvToJson(data.iv);
    return jsonObject;
}

Crypto::CryptoSettings Crypto::parseCryptoData(const QJsonObject& data) {
    CryptoSettings retData;
    retData.cipher = data["cipher"].toString();
    retData.padding = data["padding"].toString();
    retData.key = parseKeyData(data["key"].toVariant().toJsonObject());
    retData.iv = parseIvData(data["iv"].toVariant().toJsonObject());
    return retData;
}

QJsonObject Crypto::convertKeyToJson(const KeySettings& data) {
    QJsonObject jsonObject;
    jsonObject["type"] = data.type;
    QMetaEnum metaEnum = QMetaEnum::fromType<PasswordType>();
    jsonObject["passwordType"] = metaEnum.key(data.passwordType);
    jsonObject["bitsSize"] = data.bitsSize;
    jsonObject["iterations"] = data.iterations;
    jsonObject["password"] = QString(data.password);
    jsonObject["salt"] = QString(data.salt.toHex().toUpper());
    return jsonObject;
}

Crypto::KeySettings Crypto::parseKeyData(const QJsonObject& data) {
    KeySettings retData;
    retData.type = data["type"].toString();
    retData.bitsSize = data["bitsSize"].toInt();
    retData.iterations = data["iterations"].toInt();
    QMetaEnum metaEnum = QMetaEnum::fromType<PasswordType>();
    retData.passwordType = static_cast<PasswordType>(metaEnum.keyToValue(qPrintable(data["passwordType"].toString())));
    retData.password = QByteArray(data["password"].toString().toStdString());
    retData.salt = QByteArray::fromHex(QByteArray(data["salt"].toString().toStdString()));
    return retData;
}

Crypto::IvSettings Crypto::parseIvData(const QJsonObject& data) {
    IvSettings retData;
    retData.cipher = data["cipher"].toString();
    retData.bitsSize = data["bitsSize"].toInt();
    retData.value = QByteArray::fromHex(QByteArray(data["value"].toString().toStdString()));
    QMetaEnum metaEnum = QMetaEnum::fromType<IVType>();
    retData.type = static_cast<IVType>(metaEnum.keyToValue(qPrintable(data["type"].toString())));
    return retData;

}

QJsonObject Crypto::convertIvToJson(const IvSettings& data) {
    QJsonObject jsonObject;
    jsonObject["cipher"] = data.cipher;
    QMetaEnum metaEnum = QMetaEnum::fromType<IVType>();
    jsonObject["type"] = metaEnum.key(data.type);
    jsonObject["bitsSize"] = data.bitsSize;
    jsonObject["value"] = QString(data.value.toHex().toUpper());
    return jsonObject;
}

#ifndef QT_NO_DEBUG_OUTPUT

// Overload the << operator for QDebug
QDebug operator<<(QDebug dbg, const Crypto::KeySettings &settings) {
    QDebugStateSaver saver(dbg); // Saves and restores the QDebug state (e.g., space/nospace)
    dbg.setAutoInsertSpaces(true);
    dbg << "KeySettings("
        << "Type:" << settings.type << ","
        << "Password Type:" << settings.passwordType << ","
        << "Password:" << settings.password << ","
        << "Size bits:" << settings.bitsSize << ","
        << "Iterations:" << settings.iterations << ","
        << "Salt: " << settings.salt.toHex().toUpper()
        << ")";
    return dbg;
}

QDebug operator<<(QDebug dbg, const Crypto::IvSettings &settings) {
    QDebugStateSaver saver(dbg); // Saves and restores the QDebug state (e.g., space/nospace)
    dbg.setAutoInsertSpaces(true);
    dbg << "IVSettings("
        << "Type:" << settings.type << ","
        << "Cipher:"<< settings.cipher << ","
        << "Size bits:" << settings.bitsSize << ","
        << "Value:" << settings.value.toHex().toUpper()
        << ")";
    return dbg;
}

QDebug operator<<(QDebug dbg, const Crypto::CryptoSettings &settings) {
    QDebugStateSaver saver(dbg); // Saves and restores the QDebug state (e.g., space/nospace)
    dbg.setAutoInsertSpaces(true);
    dbg << "CryptoSettings("
        << "cipher:" << settings.cipher << ","
        << "padding:" << settings.padding << ","
        << "\n" << settings.key
        << "\n" << settings.iv
        << ")";
    return dbg;
}
#endif
