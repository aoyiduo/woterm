/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QKXCIPHER_H
#define QKXCIPHER_H

#include "qkxutil_private.h"

#include <QByteArray>

class KXUTIL_EXPORT QKxCipher
{
public:
    static int pkcs7PaddedLength(int dataLen, int alignSize);
    static QByteArray pkcs7Padding(const QByteArray &in, int alignSize);
    static QByteArray pkcs7UnPadding(const QByteArray &in);
    static QByteArray makeBytes(const QByteArray& pass, int cnt);
    // aes
    // key.size() == 16 || key.size() == 24 || key.size() == 32
    static bool aesEcbEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, bool enc);
    static bool aesCbcEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesCfb1Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesCfb8Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesCfb128Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesOfb128Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesCtrEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesGcmEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesXtsEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool aesOcbEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);

    // des
    static bool tripleDesEcbEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, bool enc);
    static bool tripleDesCbcEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool tripleDesCfb1Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool tripleDesCfb64Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);
    static bool tripleDesOfb64Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);

    // rc
    static bool rc4Encrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, bool enc);

    //blowfish
    static bool blowfishEcbEncrypt(const QByteArray& in, QByteArray& out, const QByteArray &key, const QByteArray& ivec, bool enc);

    static void test();
};

#endif // QKXCIPHER_H
