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
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/ssl.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <QRandomGenerator>
#define handleErrors(); ERR_print_errors_fp(stderr);
#define AES_KEY_SIZE 16  // AES-128 uses a 128-bit (16 bytes) key
#define AES_BLOCK_SIZE 16 // AES block size is 16 bytes

OSSL_PARAM* Crypto::getSSLParams() {

    sslParams.clear();

    sslParams.append(OSSL_PARAM_utf8_string((const char *) OSSL_KDF_PARAM_DIGEST, (void*) SSL_TXT_SHA256, strlen(SSL_TXT_SHA256)));
    sslParams.append(OSSL_PARAM_octet_string((const char *) OSSL_KDF_PARAM_PASSWORD, pass.data(), 16));
    sslParams.append(OSSL_PARAM_octet_string(OSSL_KDF_PARAM_SALT, salt, 16));
    sslParams.append(OSSL_PARAM_int(OSSL_KDF_PARAM_ITER, &iter));
    sslParams.append(OSSL_PARAM_END);

    return sslParams.data();
}
// void Crypto::handleErrors(void) {
//     ERR_print_errors_fp(stderr);
//     abort();
// }

Crypto::Crypto(QObject *parent)
    : QObject{parent}
{
    pass = QString("P7RNFK66");
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    EVP_KDF *kdf = EVP_KDF_fetch(NULL, OSSL_KDF_NAME_PBKDF2, NULL);
    EVP_KDF_CTX *kctx = EVP_KDF_CTX_new(kdf);

    if (!EVP_KDF_derive(kctx, key, 16, getSSLParams())) {
        handleErrors();
    }
}

QByteArray Crypto::decryptIV(QByteArray data) {
    return data;
}

QByteArray Crypto::decodeIV(unsigned char iv[]) {
    EVP_CIPHER_CTX *ctx;

    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors(); return QByteArray();
    }
    if(!EVP_CIPHER_CTX_init(ctx)) {
        handleErrors(); return QByteArray();
    }
    if(!EVP_DecryptInit_ex2(ctx, EVP_aes_128_ecb(), key, iv, getSSLParams())) {
        handleErrors(); return QByteArray();
    }
    EVP_CIPHER_CTX_set_padding(ctx, false);

    unsigned char buffer[1024], *pointer = buffer;
    int outlen=16;
    EVP_CIPHER_CTX_set_padding(ctx, false);
    if(!EVP_DecryptUpdate(ctx, pointer, &outlen, iv, 16)) {
        handleErrors();  return QByteArray();
    }
    pointer += outlen;
    if(!EVP_DecryptFinal_ex(ctx, pointer, &outlen)) {
        handleErrors();  return QByteArray();
    }
    // pointer += outlen;
    return QByteArray::fromRawData((const char *)buffer, outlen);
}

QByteArray Crypto::decrypt(QByteArray array) {
    EVP_CIPHER_CTX *ctx;
    unsigned char iv[16];
    unsigned char data[16];
    unsigned char ciphertext[128];
    int len = 16;
    if(array.size() > 16) {
        for (int i=0; i<16; i++) {
            iv[i] = array.at(i);
            data[i] = array.at(i+16);
        }
    }
    else {
       return "";
    }
/*    int iter = 16384;
            const QString pass("P7RNFK66");*/
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors();  return QByteArray();
    }

    /* Initialise the decryption operation. */
    if(!EVP_DecryptInit_ex2(ctx, EVP_aes_128_cbc(),key, NULL, getSSLParams())) {
        handleErrors();  return QByteArray();
    }

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL)) {
        handleErrors();  return QByteArray();
    }

    // if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_SET_KEY_LENGTH, 16, NULL))
    //     handleErrors();
    // unsigned char decodedIV[16];
    //unsigned char *decodedIV = (unsigned char*)decodeIV(iv).data();
    /* Initialise key and IV */

    // if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, decodedIV))
    //     handleErrors();

    //emit decoded(QString("Dec IV - ").append(
    //    QByteArray::fromRawData((const char*)decodedIV, 16).toHex(' ').toUpper()));


    /*
    unsigned char updated_iv[16];

    if (!EVP_CIPHER_CTX_get_updated_iv(ctx, updated_iv, sizeof(updated_iv))) {
        handleErrors();
    }

    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, updated_iv))
        handleErrors();
*/
    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if(!EVP_DecryptUpdate(ctx, ciphertext, &len, data, len)) {
        handleErrors();  return QByteArray();
    }

    if(!EVP_CIPHER_CTX_set_padding(ctx, false)) {
        handleErrors();  return QByteArray();
    }

    // if(!EVP_CIPHER_CTX_get_original_iv(ctx, decodedIV, 16))
    //     handleErrors();

    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    if(!EVP_DecryptFinal_ex(ctx, ciphertext + len, &len)) {
        handleErrors();  return QByteArray();
    }


    /* Clean up */
    //EVP_CIPHER_CTX_free(ctx);
    emit info(QString   ("Pass   - ").append(pass));
    emit decoded(QString("Key    - ").append(
        QByteArray::fromRawData((const char*)key, 16).toHex(' ').toUpper()));

    emit decoded(QString("IV     - ").append(
        QByteArray::fromRawData((const char*)iv, 16).toHex(' ').toUpper()));

    emit decoded(QString("Array  - ").append(
        QByteArray::fromRawData((const char*)data, len).toHex(' ').toUpper()));

    emit decoded(QString("Data   - ").append(
        QByteArray::fromRawData((const char*)ciphertext, len).toHex(' ').toUpper()));

    return QByteArray::fromRawData((const char*)ciphertext, len);
}

QByteArray Crypto::encryptIV(unsigned char iv[]) {
    // Example plaintext (16 bytes) that you want to encrypt (this simulates the 'iv' in your code)
    unsigned char ciphertext[128];
    int len, ciphertext_len;

    // Create and initialize the cipher context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handleErrors();  return QByteArray();
    }

    // Initialize the encryption operation (AES-128-ECB mode, no IV required)
    if (EVP_EncryptInit_ex2(ctx, EVP_aes_128_ecb(), key, iv, getSSLParams()) != 1) {
        handleErrors();  return QByteArray();
    }

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    // if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL))
    //     handleErrors();

    // Encrypt the data (ivToEncrypt)
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, iv, 16) != 1) {
        handleErrors();  return QByteArray();
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        handleErrors();  return QByteArray();
        EVP_CIPHER_CTX_free(ctx);
    }
    ciphertext_len += len;

    // Print the encrypted data (ciphertext) in hex
    printf("Ciphertext (hex): ");
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    // Cleanup
    EVP_CIPHER_CTX_free(ctx);

    return QByteArray::fromRawData((const char*)ciphertext, 16);
}

QByteArray Crypto::encrypt(QByteArray array) {
    EVP_CIPHER_CTX *aes;
    int len = 16;
    unsigned char ciphertext[128];
    QByteArray newArray("VOLUME_MUTE on\r\1");
    if(!(aes = EVP_CIPHER_CTX_new())) {
        handleErrors();  return QByteArray();
    }

    unsigned char ivx[16];
    unsigned char updated_iv[16];
    //unsigned char newArray[16]={ "Notify Identify\0" };
    uint buffer[16];
    QRandomGenerator::global()->fillRange(buffer);
    for(int x = 0; x < sizeof(buffer); x++) {
        ivx[x] = (unsigned char)buffer[x];
    }

    //QByteArray newiv = encryptIV(ivx);
    //if(!EVP_EncryptInit_ex(aes, EVP_aes_128_cbc(), NULL,  key, (const unsigned char*)newiv.data())) {
    if(!EVP_EncryptInit_ex(aes, EVP_aes_128_cbc(), NULL, key,(const unsigned char*) ivx)) {
        handleErrors();  return QByteArray();
    }

    if(!EVP_EncryptUpdate(aes, ciphertext, &len, (const unsigned char*)(newArray.data()), 16)) {
        handleErrors();  return QByteArray();
    }

    if (!EVP_CIPHER_CTX_get_updated_iv(aes, updated_iv, sizeof(updated_iv))) {
        handleErrors(); return QByteArray();
    }
    QByteArray newiv = QByteArray::fromRawData((const char*)updated_iv, 16);


    if(!EVP_EncryptFinal_ex(aes, ciphertext + len, &len)) {
        handleErrors();  return QByteArray();
    }
    emit info(QString   ("Pass   - ").append(pass));
    emit encoded(QString("Key    - ").append(
        QByteArray::fromRawData((const char*)key, 16).toHex(' ').toUpper()));
    emit encoded(QString("IV     - ").append(
        QByteArray::fromRawData((const char*)ivx, 16).toHex(' ').toUpper()));
    emit encoded(QString("Org    - ").append(
        QByteArray::fromRawData((const char*)newArray, newArray.length()).toHex(' ').toUpper()));
    emit encoded(QString("Up IV  - ").append(
        newiv.toHex(' ').toUpper()));
    emit encoded(QString("Data   - ").append(
        QByteArray::fromRawData((const char*)ciphertext + len, len).toHex(' ').toUpper()));

    return QByteArray::fromRawData((const char*)newiv, 16).append(QByteArray::fromRawData((const char*)ciphertext+len, len));
    // return QByteArray::fromRawData((const char*)updated_iv, 16)
    //     .append(QByteArray::fromRawData((const char*)ciphertext, 16));
}

