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

#ifndef QKXLENGTHBODYPACKET_H
#define QKXLENGTHBODYPACKET_H

#include "qkxutil_share.h"

#include <QPointer>
#include <QDataStream>


class KXUTIL_EXPORT QKxLengthBodyPacket
{
protected:
    explicit QKxLengthBodyPacket(bool bigEndian = true);
    virtual ~QKxLengthBodyPacket();
    bool packetAvailable();
    QByteArray nextPacket();
    void reset();
    void append(const QByteArray& data);
    static int writePacket(QIODevice *iodev, const QByteArray& data);
private:
    bool m_isHead;
    qint32 m_pos;
    qint32 m_length;
    QByteArray m_buffer;
    bool m_bigEndian;
};

#endif // QKXLENGTHBODYPACKET_H
