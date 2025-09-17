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
