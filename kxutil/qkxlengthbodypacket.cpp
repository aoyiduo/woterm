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

#include "qkxlengthbodypacket.h"

#include <QIODevice>
#include <QtEndian>
#include <QDebug>

#define BLOCK_SIZE      (1024 * 1024)

QKxLengthBodyPacket::QKxLengthBodyPacket(bool bigEndian)
    : m_bigEndian(bigEndian)
{
    m_buffer.reserve(1024);
    reset();
}

QKxLengthBodyPacket::~QKxLengthBodyPacket()
{

}

void QKxLengthBodyPacket::reset()
{
    m_pos = m_length = 0;
    m_isHead = true;
}

void QKxLengthBodyPacket::append(const QByteArray &data)
{
    if(m_pos > 0) {
        m_buffer.remove(0, m_pos);
        m_pos = 0;
    }
    m_buffer.append(data);
}

int QKxLengthBodyPacket::writePacket(QIODevice *iodev, const QByteArray &data)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << data;
    return iodev->write(buf);
}

bool QKxLengthBodyPacket::packetAvailable()
{
    if(m_buffer.isEmpty()) {
        return false;
    }
    if(m_isHead) {
        char *ptr = m_buffer.data() + m_pos;
        if((m_buffer.length() - m_pos) < 4) {
            m_buffer.remove(0, m_pos);
            m_pos = 0;
            return false;
        }
        qint32 *pint = (qint32*)ptr;
        m_length = m_bigEndian ? qFromBigEndian<qint32>(*pint) : qFromLittleEndian<qint32>(*pint);
        m_pos += 4;
        m_isHead = false;
        return packetAvailable();
    }
    char *ptr = m_buffer.data() + m_pos;
    if((m_buffer.length() - m_pos) < m_length) {
        m_buffer.remove(0, m_pos);
        m_pos = 0;
        return false;
    }
    return true;
}

QByteArray QKxLengthBodyPacket::nextPacket()
{
    if(m_isHead || (m_buffer.length() - m_pos) < m_length) {
        return QByteArray();
    }
    QByteArray buf = QByteArray::fromRawData(m_buffer.data() + m_pos, m_length);
    m_pos += m_length;
    m_isHead = true;
    m_length = 0;
    return buf;
}
