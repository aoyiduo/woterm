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

#ifndef QKXZIP_H
#define QKXZIP_H

#include <QObject>

class QKxZipPrivate;
class QKxZip : public QObject
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
