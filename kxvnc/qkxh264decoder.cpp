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

#include "qkxh264decoder.h"

#include <wels/codec_api.h>

#include <QLibrary>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <Windows.h>
typedef long (WINAPI *PFUNWelsCreateDecoder)(ISVCDecoder** ppDecoder);
typedef void (WINAPI *PFUNWelsDestroyDecoder)(ISVCDecoder* pDecoder);
static PFUNWelsCreateDecoder pWelsCreateDecoder = nullptr;
static PFUNWelsDestroyDecoder pWelsDestroyDecoder = nullptr;
#define LIBOPENH264_NAME        ("openh264.dll")
#else
typedef long ( *PFUNWelsCreateDecoder)(ISVCDecoder** ppDecoder);
typedef void ( *PFUNWelsDestroyDecoder)(ISVCDecoder* pDecoder);
static PFUNWelsCreateDecoder pWelsCreateDecoder = nullptr;
static PFUNWelsDestroyDecoder pWelsDestroyDecoder = nullptr;
#define LIBOPENH264_NAME        ("libopenh264.so")
#endif

#define RESOLVE(name) p##name = (PFUN##name)lib.resolve(#name);

class QKxH264DecoderPrivate
{
private:
    ISVCDecoder *m_decoder;
    SDecodingParam m_sDecParam;
public:
    explicit QKxH264DecoderPrivate(QKxH264Decoder *p) {
        m_decoder = nullptr;
#if 1
        int rv = WelsCreateDecoder(&m_decoder);
        qDebug() << "QKxH264DecoderPrivate:" << rv;
#else
        if(pWelsCreateDecoder == nullptr) {
            QString path = QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg(LIBOPENH264_NAME);
            path = QDir::cleanPath(path);
            QLibrary lib(LIBOPENH264_NAME);
            RESOLVE(WelsCreateDecoder);
            RESOLVE(WelsDestroyDecoder);
        }
        if(pWelsCreateDecoder) {
            int rv = pWelsCreateDecoder(&m_decoder);
            qDebug() << "QKxH264DecoderPrivate:" << rv;
        }
#endif
    }

    ~QKxH264DecoderPrivate() {
#if 1
        if(m_decoder) {
            m_decoder->Uninitialize();
            WelsDestroyDecoder(m_decoder);
        }
#else
        if(pWelsDestroyDecoder) {
            m_decoder->Uninitialize();
            pWelsDestroyDecoder(m_decoder);
        }
#endif
    }

    bool init(int width, int height) {
        if(m_decoder == nullptr) {
            return false;
        }
        memset(&m_sDecParam, 0, sizeof(m_sDecParam));
        m_sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
        m_sDecParam.uiTargetDqLayer = UCHAR_MAX;
        int rv = m_decoder->Initialize(&m_sDecParam);
        return rv == 0;
    }

    int decode(uchar *yuv[3], int *ystride, int *uvstride, uchar *src, int srcLength, bool reset) {
        if(m_decoder == nullptr) {
            return -1;
        }
        int rv = -1;
        SBufferInfo info;
        memset(&info, 0, sizeof(info));
        if(reset) {
            rv = m_decoder->DecodeFrame2(nullptr, 0, yuv, &info);
        }
        rv = m_decoder->DecodeFrame2(src, srcLength, yuv, &info);
        if(rv != cmResultSuccess) {
            return -1;
        }else if(info.iBufferStatus == 1){
            Q_ASSERT(info.UsrData.sSystemBuffer.iFormat == videoFormatI420);
            *ystride = info.UsrData.sSystemBuffer.iStride[0];
            *uvstride = info.UsrData.sSystemBuffer.iStride[1];
            return 1;
        }
        return 0;
    }
};

QKxH264Decoder::QKxH264Decoder(QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxH264DecoderPrivate(this);
}

QKxH264Decoder::~QKxH264Decoder()
{
    delete m_prv;
}

bool QKxH264Decoder::init(int width, int height)
{
    return m_prv->init(width, height);
}

int QKxH264Decoder::decode(uchar *yuv[3], int *ystride, int *uvstride, uchar *src, int srcLength, bool reset)
{
    return m_prv->decode(yuv, ystride, uvstride, src, srcLength, reset);
}
