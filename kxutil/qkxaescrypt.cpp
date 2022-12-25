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

#include "qkxaescrypt.h"

#include <openssl/aes.h>
#include <QDebug>
#include <QCryptographicHash>

class AesOfb {
    AES_KEY m_key;
    QByteArray m_pass;
    QByteArray m_iv;
    int m_err;
    int m_num;
public:
    AesOfb(const QByteArray& pass) {
        m_pass = pass.isEmpty() ? "AoYiDuo-20220505" : pass;
        if(pass.length() < 16) {
            int cnt = m_pass.length();
            int left = 16 - cnt;
            m_pass.append(left, 0);
        }else if(m_pass.length() > 16) {
            m_pass.resize(16);
        }
        m_iv = m_pass;
        m_num = 0;
        m_err = AES_set_encrypt_key((const unsigned char*)m_pass.data(), m_pass.size() * 8, &m_key);
    }

    void reset() {
        m_iv = m_pass;
    }

    void crypt(uchar *dst, uchar *src, quint16 cnt) {
        if(m_err != 0) {
            return;
        }
        AES_ofb128_encrypt(src, dst, cnt, &m_key, (uchar*)m_iv.data(), &m_num);
    }
};

class QKxAesOfb128CryptPrivate {
    AesOfb m_enc, m_dec;
public:
    QKxAesOfb128CryptPrivate(const QByteArray& pass)
        : m_enc(pass)
        , m_dec(pass) {
    }

    ~QKxAesOfb128CryptPrivate() {
    }

    void encrypt(uchar *dst, uchar *src, quint16 cnt) {
        m_enc.crypt(dst, src, cnt);
    }

    void decrypt(uchar *dst, uchar *src, quint16 cnt) {
        m_dec.crypt(dst, src, cnt);
    }
};

QKxAesOfb128Crypt::QKxAesOfb128Crypt(const QByteArray &pass, QObject *parent)
    : QKxAbstractCrypt(parent)
{
    m_prv = new QKxAesOfb128CryptPrivate(pass);
}

QKxAesOfb128Crypt::~QKxAesOfb128Crypt()
{
    delete m_prv;
}

void QKxAesOfb128Crypt::encrypt(uchar *dst, uchar *src, quint16 cnt)
{
    m_prv->encrypt(dst, src, cnt);
}

void QKxAesOfb128Crypt::decrypt(uchar *dst, uchar *src, quint16 cnt)
{
    m_prv->decrypt(dst, src, cnt);
}

void QKxAesOfb128Crypt::test()
{
    QByteArray my1("111111111111111111111111111111111");
    QByteArray my2("111111111111111111111111111111111");
    QByteArray my3("33333333333333333333333333333333333333333333");
    QByteArray my4("4444444444444444444444444444444444444444444");
    QByteArray my5("5555555555555555555555555555555555555555555555");
    QKxAesOfb128Crypt crypt("abc");
    QByteArray enc1 = my1, enc2 = my2, enc3 = my3, enc4 = my4, enc5 = my5;
    QByteArray dec1, dec2, dec3, dec4, dec5;
    dec1.resize(my1.length());
    dec2.resize(my2.length());
    dec3.resize(my3.length());
    dec4.resize(my4.length());
    dec5.resize(my5.length());
    crypt.encrypt((uchar*)enc1.data(), (uchar*)my1.data(), my1.length());
    crypt.encrypt((uchar*)enc2.data(), (uchar*)my2.data(), my2.length());
    crypt.encrypt((uchar*)enc3.data(), (uchar*)my3.data(), my3.length());
    crypt.encrypt((uchar*)enc4.data(), (uchar*)my4.data(), my4.length());
    crypt.encrypt((uchar*)enc5.data(), (uchar*)my5.data(), my5.length());
    crypt.decrypt((uchar*)dec1.data(), (uchar*)enc1.data(), my1.length());
    crypt.decrypt((uchar*)dec2.data(), (uchar*)enc2.data(), my2.length());
    crypt.decrypt((uchar*)dec3.data(), (uchar*)enc3.data(), my3.length());
    crypt.decrypt((uchar*)dec4.data(), (uchar*)enc4.data(), my4.length());
    crypt.decrypt((uchar*)dec5.data(), (uchar*)enc5.data(), my5.length());
    qDebug() << dec1 << dec2 << dec3 << dec4 << dec5;
}


class AesCfb {
    AES_KEY m_key;
    QByteArray m_pass;
    QByteArray m_iv;
    int m_err;
    int m_num;
public:
    AesCfb(const QByteArray& pass) {
        m_pass = pass.isEmpty() ? "AoYiDuo-20220505" : pass;
        if(pass.length() < 16) {
            int cnt = m_pass.length();
            int left = 16 - cnt;
            m_pass.append(left, 0);
        }else if(m_pass.length() > 16) {
            m_pass.resize(16);
        }
        m_iv = m_pass;
        m_num = 0;
        m_err = AES_set_encrypt_key((const unsigned char*)m_pass.data(), m_pass.size() * 8, &m_key);
    }

    void reset() {
        m_iv = m_pass;
    }

    void encrypt(uchar *dst, uchar *src, quint16 cnt) {
        if(m_err != 0) {
            return;
        }
        AES_cfb128_encrypt(src, dst, cnt, &m_key, (uchar*)m_iv.data(), &m_num, AES_ENCRYPT);
    }

    void decrypt(uchar *dst, uchar *src, quint16 cnt) {
        if(m_err != 0) {
            return;
        }
        AES_cfb128_encrypt(src, dst, cnt, &m_key, (uchar*)m_iv.data(), &m_num, AES_DECRYPT);
    }
};

class QKxAesCfb128CryptPrivate {
    AesCfb m_enc, m_dec;
public:
    QKxAesCfb128CryptPrivate(const QByteArray& pass)
        : m_enc(pass)
        , m_dec(pass) {
    }

    ~QKxAesCfb128CryptPrivate() {
    }

    void encrypt(uchar *dst, uchar *src, quint16 cnt) {
        m_enc.encrypt(dst, src, cnt);
    }

    void decrypt(uchar *dst, uchar *src, quint16 cnt) {
        m_dec.decrypt(dst, src, cnt);
    }
};

QKxAesCfb128Crypt::QKxAesCfb128Crypt(const QByteArray &pass, QObject *parent)
    : QKxAbstractCrypt(parent)
{
    m_prv = new QKxAesCfb128CryptPrivate(pass);
}

QKxAesCfb128Crypt::~QKxAesCfb128Crypt()
{
    delete m_prv;
}

void QKxAesCfb128Crypt::encrypt(uchar *dst, uchar *src, quint16 cnt)
{
    m_prv->encrypt(dst, src, cnt);
}

void QKxAesCfb128Crypt::decrypt(uchar *dst, uchar *src, quint16 cnt)
{
    m_prv->decrypt(dst, src, cnt);
}

void QKxAesCfb128Crypt::test()
{
    QByteArray my1("111111111111111111111111111111111");
    QByteArray my2("111111111111111111111111111111111");
    QByteArray my3("33333333333333333333333333333333333333333333");
    QByteArray my4("4444444444444444444444444444444444444444444");
    QByteArray my5("5555555555555555555555555555555555555555555555");
    QKxAesOfb128Crypt crypt("abc");
    QByteArray enc1 = my1, enc2 = my2, enc3 = my3, enc4 = my4, enc5 = my5;
    QByteArray dec1, dec2, dec3, dec4, dec5;
    dec1.resize(my1.length());
    dec2.resize(my2.length());
    dec3.resize(my3.length());
    dec4.resize(my4.length());
    dec5.resize(my5.length());
    crypt.encrypt((uchar*)enc1.data(), (uchar*)my1.data(), my1.length());
    crypt.encrypt((uchar*)enc2.data(), (uchar*)my2.data(), my2.length());
    crypt.encrypt((uchar*)enc3.data(), (uchar*)my3.data(), my3.length());
    crypt.encrypt((uchar*)enc4.data(), (uchar*)my4.data(), my4.length());
    crypt.encrypt((uchar*)enc5.data(), (uchar*)my5.data(), my5.length());
    crypt.decrypt((uchar*)dec1.data(), (uchar*)enc1.data(), my1.length());
    crypt.decrypt((uchar*)dec2.data(), (uchar*)enc2.data(), my2.length());
    crypt.decrypt((uchar*)dec3.data(), (uchar*)enc3.data(), my3.length());
    crypt.decrypt((uchar*)dec4.data(), (uchar*)enc4.data(), my4.length());
    crypt.decrypt((uchar*)dec5.data(), (uchar*)enc5.data(), my5.length());
    qDebug() << dec1 << dec2 << dec3 << dec4 << dec5;
}
