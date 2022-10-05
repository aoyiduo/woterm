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

#ifndef QKXRC4CRYPT_H
#define QKXRC4CRYPT_H

#include "qkxutil_share.h"

#include <QtGlobal>
#include <QByteArray>


class QKxRC4CryptPrivate;
class KXUTIL_EXPORT QKxRC4Crypt
{
public:
    explicit QKxRC4Crypt(const QByteArray& pass);
    ~QKxRC4Crypt();
    void encrypt(QByteArray& dst, const QByteArray& src);
    void decrypt(QByteArray& dst, const QByteArray& src);
    static void test();
private:
    QKxRC4CryptPrivate *m_prv;
};

#endif // QKXRC4CRYPT_H
