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

#ifndef QKXABSTRACTCRYPT_H
#define QKXABSTRACTCRYPT_H

#include "qkxutil_share.h"
#include <QObject>

class KXUTIL_EXPORT QKxAbstractCrypt : public QObject
{
    Q_OBJECT
public:
    explicit QKxAbstractCrypt(QObject *parent = nullptr);
    ~QKxAbstractCrypt();
    virtual void encrypt(uchar *dst, uchar* src, quint16 cnt) = 0;
    virtual void decrypt(uchar *dst, uchar* src, quint16 cnt) = 0;
};

#endif // QKXABSTRACTCRYPT_H
