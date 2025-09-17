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

#ifndef QKXAESCRYPT_H
#define QKXAESCRYPT_H

#include "qkxabstractcrypt.h"

class QKxAesOfb128CryptPrivate;
class KXUTIL_EXPORT QKxAesOfb128Crypt : public QKxAbstractCrypt
{
public:
    explicit QKxAesOfb128Crypt(const QByteArray& pass, QObject *parent=nullptr);
    ~QKxAesOfb128Crypt();
    void encrypt(uchar *dst, uchar *src, quint16 cnt);
    void decrypt(uchar *dst, uchar *src, quint16 cnt);
    static void test();
private:
    QKxAesOfb128CryptPrivate *m_prv;
};

class QKxAesCfb128CryptPrivate;
class KXUTIL_EXPORT QKxAesCfb128Crypt : public QKxAbstractCrypt
{
public:
    explicit QKxAesCfb128Crypt(const QByteArray& pass, QObject *parent=nullptr);
    ~QKxAesCfb128Crypt();
    void encrypt(uchar *dst, uchar *src, quint16 cnt);
    void decrypt(uchar *dst, uchar *src, quint16 cnt);
    static void test();
private:
    QKxAesCfb128CryptPrivate *m_prv;
};

#endif // QKXXORCRYPT_H
