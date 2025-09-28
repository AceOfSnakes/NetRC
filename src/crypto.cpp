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

Crypto::Crypto(QObject *parent)
    : QObject{parent}
{
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    const char *pass = "P7RNFK66";
    //const char *pass = "12345678";
    int iter = 16384;

    EVP_KDF *kdf = EVP_KDF_fetch(NULL, OSSL_KDF_NAME_PBKDF2, NULL);
    EVP_KDF_CTX *kctx = EVP_KDF_CTX_new(kdf);
    OSSL_PARAM params[] = {
                           OSSL_PARAM_utf8_string(OSSL_KDF_PARAM_DIGEST, (void *)SSL_TXT_SHA256, 0),
                           OSSL_PARAM_octet_string(OSSL_KDF_PARAM_PASSWORD, (void *)pass, strlen(pass)),
                           OSSL_PARAM_octet_string(OSSL_KDF_PARAM_SALT, salt, 16),
                           OSSL_PARAM_int(OSSL_KDF_PARAM_ITER, &iter),
                           OSSL_PARAM_END};

    if (EVP_KDF_derive(kctx, key, 16, params) <= 0) {
        qDebug() << "EVP_KDF_derive failed";
    }
}

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

QByteArray Crypto::decryptIV(QByteArray data) {
    return data;

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
    const char *pass = "P7RNFK66";
    //const char *pass = "12345678";
    int iter = 16384;
    OSSL_PARAM params[] = {
                           OSSL_PARAM_utf8_string(OSSL_KDF_PARAM_DIGEST, (void *)SSL_TXT_SHA256, 0),
                           OSSL_PARAM_octet_string(OSSL_KDF_PARAM_PASSWORD, (void *)pass, strlen(pass)),
                           OSSL_PARAM_octet_string(OSSL_KDF_PARAM_SALT, salt, 16),
                           OSSL_PARAM_int(OSSL_KDF_PARAM_ITER, &iter),
                           OSSL_PARAM_END};
/*    int iter = 16384;
            const QString pass("P7RNFK66");*/
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /* Initialise the decryption operation. */
    if(!EVP_DecryptInit_ex2(ctx, EVP_aes_128_cbc(),NULL, NULL, params))
        handleErrors();

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 16, NULL))
         handleErrors();

    // if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_SET_KEY_LENGTH, 16, NULL))
    //     handleErrors();

    /* Initialise key and IV */
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();
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
    if(!EVP_DecryptUpdate(ctx, ciphertext, &len, data, len))
        handleErrors();

    if(!EVP_CIPHER_CTX_set_padding(ctx, false))
        handleErrors();
    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    if(!EVP_DecryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();


    /* Clean up */
    //EVP_CIPHER_CTX_free(ctx);

    return QByteArray::fromRawData((const char*)ciphertext, len);
}

QByteArray Crypto::encrypt(QByteArray array) {
    EVP_CIPHER_CTX *aes;
    int len = 16;
    unsigned char ciphertext[128];
    if(!(aes = EVP_CIPHER_CTX_new())) {
        handleErrors();
    }
    unsigned char iv[16];
    uint buffer[16];
    QRandomGenerator::global()->fillRange(buffer);
    for(int x = 0; x < sizeof(buffer); x++) {
        iv[x] = (unsigned char)buffer[x];
    }

    unsigned char updated_iv[16];

    if(!EVP_EncryptInit_ex(aes, EVP_aes_128_cbc(), NULL,  key, iv)) {
        handleErrors();
    }

    if(!EVP_EncryptUpdate(aes, ciphertext, &len, (const unsigned char*)(array.data()), 16)) {
        handleErrors();
    }

    if (!EVP_CIPHER_CTX_get_updated_iv(aes, updated_iv, sizeof(updated_iv))) {
        handleErrors();
    }
    if(!EVP_EncryptFinal_ex(aes, ciphertext + len, &len)) {
        handleErrors();
    }
    return QByteArray::fromRawData((const char*)updated_iv, 16)
        .append(QByteArray::fromRawData((const char*)ciphertext, 16));
}

