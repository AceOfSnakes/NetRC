#include "crypto.h"
#include <openssl/sha.h>
#include <openssl/evp.h>

typedef unsigned char byte;

Crypto::Crypto(QObject *parent)
    : QObject{parent}
{
    /*EVP_MD_CTX ctx;
    const int DataLen = 30;
    byte digest[EVP_MAX_MD_SIZE];
    unsigned int outLen;
    int i;
    byte* testdata = (byte *)malloc(DataLen);

    for (i=0; i<DataLen; i++) testdata[i] = 0;

    EVP_DigestInit(&ctx, EVP_sha256());
    EVP_DigestUpdate(&ctx, testdata, DataLen);
    EVP_DigestFinal(&ctx, digest, &outLen);*/
}

QByteArray Crypto::decrypt(QByteArray& array) {
    return array;
}
QByteArray Crypto::encrypt(QByteArray& array) {
    return array;
}

