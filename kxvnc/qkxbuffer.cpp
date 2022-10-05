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

#include "qkxbuffer.h"
#include "qkxutils.h"

#include <QDateTime>
#include <QtEndian>
#include <QDebug>



QKxBuffer::QKxBuffer(const QByteArray &buf)
    : m_buf(buf)
{
    m_pbuf = m_buf.data();
}

QByteArray QKxBuffer::readArray(int timeout)
{
    int len = readInt32(timeout);
    QByteArray buf(m_pbuf, len);
    m_pbuf += len;
    return buf;
}

qint32 QKxBuffer::readInt32(int timeout)
{
    qint32 v = *(qint32*)m_pbuf;
    m_pbuf += 4;
    return qFromBigEndian<qint32>(v);
}

quint32 QKxBuffer::readUint32(int timeout)
{
    quint32 v = *(quint32*)m_pbuf;
    m_pbuf += 4;
    return qFromBigEndian<quint32>(v);
}

qint16 QKxBuffer::readInt16(int timeout)
{
    qint16 v = *(qint16*)m_pbuf;
    m_pbuf += 2;
    return qFromBigEndian<quint16>(v);
}

quint16 QKxBuffer::readUint16(int timeout)
{
    quint16 v = *(quint16*)m_pbuf;
    m_pbuf += 2;
    return qFromBigEndian<quint16>(v);
}

qint8 QKxBuffer::readInt8(int timeout)
{
    qint8 v = *(qint8*)m_pbuf;
    m_pbuf += 1;
    return v;
}

quint8 QKxBuffer::readUint8(int timeout)
{
    quint8 v = *(quint8*)m_pbuf;
    m_pbuf += 1;
    return v;
}

quint32 QKxBuffer::readRgb32(int timeout)
{
    quint32 v = *(quint32*)m_pbuf;
    m_pbuf += 4;
    return v;
}

quint32 QKxBuffer::readRgb24(int timeout)
{
    quint32 r = (quint8)m_pbuf[0];
    quint32 g = (quint8)m_pbuf[1];
    quint32 b = (quint8)m_pbuf[2];
    m_pbuf += 3;
    return (b << 16) | (g << 8) | r;
}

quint16 QKxBuffer::readRgb16(int timeout)
{
    quint16 v = *(quint16*)m_pbuf;
    m_pbuf += 2;
    return v;
}

quint8 QKxBuffer::readRgb8(int timeout)
{
    quint8 v = *(quint8*)m_pbuf;
    m_pbuf += 1;
    return v;
}

void QKxBuffer::readSkip(int len, int timeout)
{
    m_pbuf += len;
}

char *QKxBuffer::current(int len, int timeout)
{
    return m_pbuf;
}
