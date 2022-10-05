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

private:
    bool m_isHead;
    qint32 m_pos;
    qint32 m_length;
    QByteArray m_buffer;
    bool m_bigEndian;
};

#endif // QKXLENGTHBODYPACKET_H
