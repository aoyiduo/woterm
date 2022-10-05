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

#include "qkxzip.h"

#include <zlib.h>

class QKxZipPrivate {
private:
    QKxZip *m_pub;
    z_stream m_zenc;
    z_stream m_zdec;
public:
    explicit QKxZipPrivate(QKxZip *pub)
        : m_pub(pub) {
        memset(&m_zenc, 0, sizeof(z_stream));
        //deflateInit(&m_zenc, Z_DEFAULT_COMPRESSION);
        int err = deflateInit2(&m_zenc, 5, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        memset(&m_zdec, 0, sizeof(z_stream));
        int err2 = inflateInit(&m_zdec);
    }

    ~QKxZipPrivate() {
        deflateEnd(&m_zenc);
        inflateEnd(&m_zdec);
    }

    int encode(const QByteArray& data, QByteArray &result) {
        m_zenc.avail_in = data.length();
        m_zenc.next_in = (uchar*)data.constData();
        m_zenc.total_in = m_zenc.total_out = 0;

        QByteArray &buf = result;
        int pos = buf.length();
        /* server site must be Z_SYNC_FLUSH
         * client site can be either Z_SYNC_FLUSH or Z_NO_FLUSH.
        */
        int ret;
        int bufcnt = data.length();
        if(bufcnt < 1024) {
            bufcnt = 1024;
        }
        do{
            buf.resize(pos + m_zenc.total_out + bufcnt);
            char *pdst = buf.data() + m_zenc.total_out + pos;
            m_zenc.avail_out = bufcnt;
            m_zenc.next_out = (uchar*)pdst;
            ret = deflate(&m_zenc, Z_SYNC_FLUSH);
            if(ret != Z_OK) {
                return -1;
            }
        }while(m_zenc.avail_out == 0);
        int total = pos + m_zenc.total_out;
        buf.resize(total);
        return m_zenc.total_out;
    }

    int decode(const QByteArray& data, QByteArray &result) {
        m_zdec.avail_in = data.length();
        m_zdec.next_in = (uchar*)data.constData();
        m_zdec.total_in = m_zdec.total_out = 0;

        QByteArray &buf = result;
        int pos = buf.length();
        /* server site must be Z_SYNC_FLUSH
         * client site can be either Z_SYNC_FLUSH or Z_NO_FLUSH.
        */
        int ret;
        int bufcnt = data.length();
        if(bufcnt < 10240) {
            bufcnt = 10240;
        }
        do{
            buf.resize(pos + m_zdec.total_out + bufcnt);
            char *pdst = buf.data() + m_zdec.total_out + pos;
            m_zdec.avail_out = bufcnt;
            m_zdec.next_out = (uchar*)pdst;
            ret = inflate(&m_zdec, Z_SYNC_FLUSH);
            if( ret != Z_OK) {
                return -1;
            }
        }while(m_zdec.avail_out == 0);
        int total = pos + m_zdec.total_out;
        buf.resize(total);
        return m_zdec.total_out;
    }
};

QKxZip::QKxZip(QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxZipPrivate(this);
}

QKxZip::~QKxZip()
{
    delete m_prv;
}

int QKxZip::encode(const QByteArray &data, QByteArray &result)
{
    return m_prv->encode(data, result);
}

int QKxZip::decode(const QByteArray &data, QByteArray &result)
{
    return m_prv->decode(data, result);
}
