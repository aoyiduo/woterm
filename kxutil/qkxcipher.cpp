/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxcipher.h"

#include <openssl/modes.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/rc4.h>
#include <openssl/blowfish.h>

#include <QDebug>

int QKxCipher::pkcs7PaddedLength(int dataLen, int alignSize)
{
    int remainder = dataLen % alignSize;
    int paddingSize = (remainder == 0) ? alignSize : (alignSize - remainder);
    return (dataLen + paddingSize);
}

QByteArray QKxCipher::pkcs7Padding(const QByteArray &in, int alignSize)
{
    int remainder = in.size() % alignSize;
    int paddingSize = (remainder == 0) ? alignSize : (alignSize - remainder);

    QByteArray temp(in);
    temp.append(paddingSize, paddingSize);
    return temp;
}

QByteArray QKxCipher::pkcs7UnPadding(const QByteArray &in)
{
    char paddingSize = in.at(in.size() - 1);
    return in.left(in.size() - paddingSize);
}

QByteArray QKxCipher::makeBytes(const QByteArray &pass, int cnt)
{
    QByteArray tmp = pass;
    if(pass.length() < cnt) {
        int left = cnt - pass.length();
        tmp.append(left, '0');
    }else if(pass.length() > cnt) {
        tmp.resize(cnt);
    }
    return tmp;
}

QByteArray QKxCipher::alignBytes(const QByteArray &in, int alignSize)
{
    int remainder = in.size() % alignSize;
    if(remainder > 0) {
        QByteArray temp(in);
        int paddingSize = alignSize - remainder;
        temp.append(paddingSize, paddingSize);
        return temp;
    }
    return in;
}

bool QKxCipher::isAlign(const QByteArray &in, int alignSize)
{
    return (in.size() % alignSize) == 0;
}

bool QKxCipher::aesEcbEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if (enc) {
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
            return false;
        }
        QByteArray inTemp = pkcs7Padding(in, AES_BLOCK_SIZE);
        out.resize(inTemp.size());
        for (int i = 0; i < inTemp.size() / AES_BLOCK_SIZE; i++) {
            AES_ecb_encrypt((const unsigned char*)inTemp.data() + AES_BLOCK_SIZE * i,
                            (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                            &aes_key,
                            AES_ENCRYPT);
        }
    } else {
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
            return false;
        }

        out.resize(in.size());
        for (int i = 0; i < in.size() / AES_BLOCK_SIZE; i++) {
            AES_ecb_encrypt((const unsigned char*)in.data() + AES_BLOCK_SIZE * i,
                            (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                            &aes_key,
                            AES_DECRYPT);
        }
        out = pkcs7UnPadding(out);
    }
    return true;
}

bool QKxCipher::aesCbcEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }
    if (enc) {
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
            return false;
        }
        QByteArray inTemp = pkcs7Padding(in, AES_BLOCK_SIZE);
        QByteArray ivecTemp = ivec; // ivec will be changed.
        out.resize(inTemp.size());
        AES_cbc_encrypt((const unsigned char*)inTemp.data(),
                        (unsigned char*)out.data(),
                        inTemp.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        AES_ENCRYPT);
    } else {
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
            return false;
        }

        QByteArray ivecTemp = ivec; // ivec will be changed after encrypt.
        out.resize(in.size());
        AES_cbc_encrypt((const unsigned char*)in.data(),
                        (unsigned char*)out.data(),
                        in.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        AES_DECRYPT);

        // remove PKCS7Padding
        out = pkcs7UnPadding(out);
    }
    return true;
}

bool QKxCipher::aesCfb1Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    AES_KEY aes_key;
    if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
        return false;
    }

    int num = 0;
    QByteArray ivecTemp = ivec; // ivec
    int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
    out.resize(in.size()); //
    AES_cfb1_encrypt((const unsigned char*)in.data(),
                    (unsigned char*)out.data(),
                    in.size() * 8,
                    &aes_key,
                    (unsigned char*)ivecTemp.data(),
                    &num,
                    encVal);
    return true;
}

bool QKxCipher::aesCfb8Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    AES_KEY aes_key;
    if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
        return false;
    }

    int num = 0;
    QByteArray ivecTemp = ivec;
    int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
    out.resize(in.size());
    AES_cfb8_encrypt((const unsigned char*)in.data(),
                    (unsigned char*)out.data(),
                    in.size(),
                    &aes_key,
                    (unsigned char*)ivecTemp.data(),
                    &num,
                    encVal);
    return true;
}

bool QKxCipher::aesCfb128Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    AES_KEY aes_key;
    if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
        return false;
    }

    int num = 0;
    QByteArray ivecTemp = ivec;
    int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
    out.resize(in.size());
    AES_cfb128_encrypt((const unsigned char*)in.data(),
                    (unsigned char*)out.data(),
                    in.size(),
                    &aes_key,
                    (unsigned char*)ivecTemp.data(),
                    &num,
                    encVal);
    return true;
}

bool QKxCipher::aesOfb128Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    AES_KEY aes_key;
    if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0) {
        return false;
    }

    Q_UNUSED(enc);

    int num = 0;
    QByteArray ivecTemp = ivec;
    out.resize(in.size());
    AES_ofb128_encrypt((const unsigned char*)in.data(),
                    (unsigned char*)out.data(),
                    in.size(),
                    &aes_key,
                    (unsigned char*)ivecTemp.data(),
                    &num);
    return true;
}



static bool EvpEncrypt(EVP_CIPHER_CTX *ctx, const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, const EVP_CIPHER *ciper, bool enc){
    if (enc) {
        int ret = EVP_EncryptInit_ex(ctx, ciper, NULL, (const unsigned char*)key.data(), (const unsigned char*)ivec.data());
        if(ret != 1) {
            return false;
        }

        int mlen = 0;
        out.fill(0, in.size() + AES_BLOCK_SIZE);
        ret = EVP_EncryptUpdate(ctx, (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), in.size());
        if(ret != 1) {
            return false;
        }

        int flen = 0;
        ret = EVP_EncryptFinal_ex(ctx, (unsigned char *)out.data() + mlen, &flen);
        if(ret != 1) {
            return false;
        }
        out.resize(mlen + flen);
        return true;
    } else {
        int ret = EVP_DecryptInit_ex(ctx, ciper, NULL, (const unsigned char*)key.data(), (const unsigned char*)ivec.data());
        if(ret != 1) {
            return false;
        }
        int mlen = 0;
        out.fill(0, in.size());
        ret = EVP_DecryptUpdate(ctx, (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), in.size());
        if(ret != 1) {
            return false;
        }
        int flen = 0;
        // https://blog.csdn.net/anda0109/article/details/51281070
        // This instruction only needs to be called when the bytes are not aligned to 16 bytes.
        if((in.length() % AES_BLOCK_SIZE) != 0) {
            ret = EVP_DecryptFinal_ex(ctx, (unsigned char *)out.data() + mlen, &flen);
            if(ret != 1) {
                return false;
            }
        }
        out.resize(mlen + flen);
        return true;
    }
}

bool QKxCipher::aesCtrEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    if(!isAlign(in, 16)) {
        return false;
    }

    const EVP_CIPHER * cipher = nullptr;
    if (key.size() == 16) {
        cipher = EVP_aes_128_ctr();
    } else if (key.size() == 24) {
        cipher = EVP_aes_192_ctr();
    } else {
        cipher = EVP_aes_256_ctr();
    }
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    bool on = EvpEncrypt(ctx, in, out, key, ivec, cipher, enc);
    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
    return on;
}

bool QKxCipher::aesGcmEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(!(ivec.size() == 16 || ivec.size() == 12)) {
        return false;
    }
    if(!isAlign(in, 16)) {
        return false;
    }

    const EVP_CIPHER * cipher = nullptr;
    if (key.size() == 16) {
        cipher = EVP_aes_128_gcm();
    } else if (key.size() == 24) {
        cipher = EVP_aes_192_gcm();
    } else {
        cipher = EVP_aes_256_gcm();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    bool on= EvpEncrypt(ctx, in, out, key, ivec, cipher, enc);
    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
    return on;
}

bool QKxCipher::aesXtsEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }
    if(!isAlign(in, 16)) {
        return false;
    }

    const EVP_CIPHER * cipher = nullptr;
    if (key.size() == 16) {
        cipher = EVP_aes_128_xts();
    } else {
        cipher = EVP_aes_256_xts();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    bool on = EvpEncrypt(ctx, in, out, key, ivec, cipher, enc);
    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
    return on;
}

bool QKxCipher::aesOcbEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(!(key.size() == 16 || key.size() == 24 || key.size() == 32)) {
        return false;
    }
    if(ivec.size() != 16) {
        return false;
    }

    const EVP_CIPHER * cipher = nullptr;
    if (key.size() == 16) {
        cipher = EVP_aes_128_ocb();
    } else if (key.size() == 24) {
        cipher = EVP_aes_192_ocb();
    } else {
        cipher = EVP_aes_256_ocb();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    bool on = EvpEncrypt(ctx, in, out, key, ivec, cipher, enc);
    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
    return on;
}


const int DES_BLOCK_SIZE = 8;

static void setKey(const QByteArray &key, DES_key_schedule &sch1, DES_key_schedule &sch2, DES_key_schedule &sch3)
{
    const_DES_cblock key1, key2, key3;
    memcpy(key1, key.data(), 8);
    memcpy(key2, key.data() + 8, 8);
    memcpy(key3, key.data() + 16, 8);

    DES_set_key_unchecked(&key1, &sch1);
    DES_set_key_unchecked(&key2, &sch2);
    DES_set_key_unchecked(&key3, &sch3);
}



bool QKxCipher::tripleDesEcbEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool enc)
{
    if(key.size() != 24) {
        return false;
    }

    DES_key_schedule sch1, sch2, sch3;
    setKey(key, sch1, sch2, sch3);
    if (enc) {
        QByteArray inTemp = pkcs7Padding(in, DES_BLOCK_SIZE);
        out.resize(inTemp.size());
        for (int i = 0; i < inTemp.size() / DES_BLOCK_SIZE; i++) {
            DES_ecb3_encrypt((const_DES_cblock*)(inTemp.constData() + i * DES_BLOCK_SIZE),
                             (DES_cblock *)(out.data() + i * DES_BLOCK_SIZE),
                             &sch1, &sch2, &sch3, DES_ENCRYPT);
        }
    } else {
        out.resize(in.size());
        for (int i = 0; i < in.size() / DES_BLOCK_SIZE; i++) {
            DES_ecb3_encrypt((const_DES_cblock*)(in.constData() + i * DES_BLOCK_SIZE),
                             (DES_cblock *)(out.data() + i * DES_BLOCK_SIZE),
                             &sch1, &sch2, &sch3, DES_DECRYPT);
        }
        out = pkcs7UnPadding(out);
    }
    return true;
}

bool QKxCipher::tripleDesCbcEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(key.size() != 24) {
        return false;
    }
    if(ivec.size() != 8) {
        return false;
    }
    DES_key_schedule sch1, sch2, sch3;
    setKey(key, sch1, sch2, sch3);

    QByteArray ivecTemp = ivec;
    if (enc) {
        QByteArray inTemp = pkcs7Padding(in, DES_BLOCK_SIZE);
        out.resize(inTemp.size());
        DES_ede3_cbc_encrypt((const unsigned char *)inTemp.constData(),
                             (unsigned char *)out.data(),
                             inTemp.size(), &sch1, &sch2, &sch3,
                             (DES_cblock *)ivecTemp.data(), DES_ENCRYPT);
    } else {
        out.resize(in.size());
        DES_ede3_cbc_encrypt((const unsigned char *)in.constData(),
                             (unsigned char *)out.data(),
                             in.size(), &sch1, &sch2, &sch3,
                             (DES_cblock *)ivecTemp.data(), DES_DECRYPT);

        out = pkcs7UnPadding(out);
    }
    return true;
}

bool QKxCipher::tripleDesCfb1Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(key.size() != 24) {
        return false;
    }
    if(ivec.size() != 8) {
        return false;
    }

    DES_key_schedule sch1, sch2, sch3;
    setKey(key, sch1, sch2, sch3);

    QByteArray ivecTemp = ivec;
    int encVal = enc ? DES_ENCRYPT : DES_DECRYPT;
    out.resize(in.size());
    DES_ede3_cfb_encrypt((const unsigned char *)in.constData(),
                         (unsigned char *)out.data(),
                         8, in.size(), &sch1, &sch2, &sch3,
                         (DES_cblock *)ivecTemp.data(), encVal);
    return true;
}

bool QKxCipher::tripleDesCfb64Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(key.size() != 24) {
        return false;
    }
    if(ivec.size() != 8) {
        return false;
    }

    DES_key_schedule sch1, sch2, sch3;
    setKey(key, sch1, sch2, sch3);

    int num = 0;
    QByteArray ivecTemp = ivec;
    int encVal = enc ? DES_ENCRYPT : DES_DECRYPT;
    out.resize(in.size());
    DES_ede3_cfb64_encrypt((const unsigned char *)in.constData(),
                         (unsigned char *)out.data(),
                         in.size(), &sch1, &sch2, &sch3,
                         (DES_cblock *)ivecTemp.data(), &num, encVal);
    return true;
}

bool QKxCipher::tripleDesOfb64Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray &ivec, bool enc)
{
    if(key.size() != 24) {
        return false;
    }
    if(ivec.size() != 8) {
        return false;
    }

    Q_UNUSED(enc)

    DES_key_schedule sch1, sch2, sch3;
    setKey(key, sch1, sch2, sch3);

    int num = 0;
    QByteArray ivecTemp = ivec;
    out.resize(in.size());
    DES_ede3_ofb64_encrypt((const unsigned char *)in.constData(),
                         (unsigned char *)out.data(),
                         in.size(), &sch1, &sch2, &sch3,
                         (DES_cblock *)ivecTemp.data(), &num);
    return true;
}

bool QKxCipher::rc4Encrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool enc)
{
    RC4_KEY enkey;

    RC4_set_key(&enkey, key.length(), (unsigned char*)key.data());
    out.resize(in.length());
    RC4(&enkey, in.length(), (const unsigned char*)in.data(), (unsigned char*)out.data());
    return true;
}

bool QKxCipher::blowfishEcbEncrypt(const QByteArray &in, QByteArray &out, const QByteArray &key, const QByteArray& ivec, bool enc) {
    if(ivec.length() != 8) {
        return false;
    }
    if(!isAlign(in, 8)) {
        return false;
    }
    if(enc) {
        BF_KEY dekey;
        BF_set_key(&dekey, key.length(), (const unsigned char*)key.data());
        out.resize(in.length());
        QByteArray tmpVec = ivec;
        BF_cbc_encrypt((const unsigned char*)in.data(), (unsigned char*)out.data(), (long)in.length(), &dekey, (unsigned char*)tmpVec.data(), BF_ENCRYPT);
    }else{
        BF_KEY dekey;
        BF_set_key(&dekey, key.length(), (const unsigned char*)key.data());
        out.resize(in.length());
        QByteArray tmpVec = ivec;
        BF_cbc_encrypt((const unsigned char*)in.data(), (unsigned char*)out.data(), (long)in.length(), &dekey, (unsigned char*)tmpVec.data(), BF_DECRYPT);
    }
    return true;
}

void QKxCipher::test()
{
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesEcbEncrypt(in, out, key, true);
        aesEcbEncrypt(out, out2, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesCbcEncrypt(in, out, key, key, true);
        aesCbcEncrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesCfb1Encrypt(in, out, key, key, true);
        aesCfb1Encrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesCfb8Encrypt(in, out, key, key, true);
        aesCfb8Encrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesCfb128Encrypt(in, out, key, key, true);
        aesCfb128Encrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesOfb128Encrypt(in, out, key, key, true);
        aesOfb128Encrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesCtrEncrypt(in, out, key, key, true);
        aesCtrEncrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        in.resize(48);
        aesGcmEncrypt(in, out, key, key, true);
        aesGcmEncrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesXtsEncrypt(in, out, key, key, true);
        aesXtsEncrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        aesOcbEncrypt(in, out, key, key, true);
        aesOcbEncrypt(out, out2, key, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 24);
        tripleDesEcbEncrypt(in, out, key, true);
        tripleDesEcbEncrypt(out, out2, key, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123", ivec="123";
        key = makeBytes(key, 24);
        ivec = makeBytes(ivec, 8);
        tripleDesCbcEncrypt(in, out, key, ivec, true);
        tripleDesCbcEncrypt(out, out2, key, ivec, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123", ivec="123";
        key = makeBytes(key, 24);
        ivec = makeBytes(ivec, 8);
        tripleDesCfb1Encrypt(in, out, key, ivec, true);
        tripleDesCfb1Encrypt(out, out2, key, ivec, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123", ivec="123";
        key = makeBytes(key, 24);
        ivec = makeBytes(ivec, 8);
        tripleDesCfb64Encrypt(in, out, key, ivec, true);
        tripleDesCfb64Encrypt(out, out2, key, ivec, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123", ivec="123";
        key = makeBytes(key, 24);
        ivec = makeBytes(ivec, 8);
        tripleDesOfb64Encrypt(in, out, key, ivec, true);
        tripleDesOfb64Encrypt(out, out2, key, ivec, false);
        Q_ASSERT(in == out2);
    }
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
        QByteArray out, out2, key = "123";
        key = makeBytes(key, 16);
        rc4Encrypt(in, out, key, true);
        rc4Encrypt(out, out2, key, false);
        Q_ASSERT(in == out2);
    }
    if(true)
    {
        QByteArray in = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz01234567";
        QByteArray out, out2, key = "123", ivec="123";
        key = makeBytes(key, 16);
        ivec = makeBytes(ivec, 8);
        blowfishEcbEncrypt(QKxCipher::pkcs7Padding(in, 8), out, key, ivec, true);
        blowfishEcbEncrypt(out, out2, key, ivec, false);
        QByteArray my = QKxCipher::pkcs7UnPadding(out2);
        Q_ASSERT(in == my);
    }
}
