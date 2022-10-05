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

#ifndef QKXBUFFER_H
#define QKXBUFFER_H

#include "qkxreader.h"

class QKxBuffer: public QKxReader
{
public:
    QKxBuffer(const QByteArray& buf);
    virtual QByteArray readArray(int timeout = 10000);
    virtual qint32 readInt32(int timeout = 10000);
    virtual quint32 readUint32(int timeout = 10000);
    virtual qint16 readInt16(int timeout = 10000);
    virtual quint16 readUint16(int timeout = 10000);
    virtual qint8 readInt8(int timeout = 10000);
    virtual quint8 readUint8(int timeout = 10000);
    virtual quint32 readRgb32(int timeout = 10000);
    virtual quint32 readRgb24(int timeout = 10000);
    virtual quint16 readRgb16(int timeout = 10000);
    virtual quint8 readRgb8(int timeout = 10000);
    virtual void readSkip(int len, int timeout = 10000);
    virtual char* current(int len, int timeout = 10000);
private:
    QByteArray m_buf;
    char* m_pbuf;
};

#endif // QKXBUFFER_H
