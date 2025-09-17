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

#ifndef QKXZIP_H
#define QKXZIP_H

#include "qkxutil_share.h"

#include <QObject>

class QKxZipPrivate;
class KXUTIL_EXPORT QKxZip : public QObject
{
    Q_OBJECT
public:
    explicit QKxZip(QObject *parent = 0);
    ~QKxZip();

    int encode(const QByteArray& data, QByteArray &result);
    int decode(const QByteArray& data, QByteArray &result);
private:
    QKxZipPrivate *m_prv;
};

#endif // QKXZIP_H
