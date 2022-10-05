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

#include "qkxopusdecoder.h"

#include <opus/opus.h>

#include <QDataStream>
#include <QFile>

#define MAX_PACKET  (1500)

class QKxOpusDecoderPrivate {
    QKxOpusDecoder *m_p;
    OpusDecoder *m_dec;
    int m_channel;
    int m_sampleRate;
public:
    QKxOpusDecoderPrivate(int sampleRate, int channel, QKxOpusDecoder *p)
        : m_p(p){
        int err = 0;
        m_channel = channel;
        m_sampleRate = sampleRate;
        m_dec = opus_decoder_create(sampleRate, channel, &err);
    }

    ~QKxOpusDecoderPrivate() {
        if(m_dec != nullptr) {
            opus_decoder_destroy(m_dec);
        }
    }

    QByteArray decode(const QByteArray& sample) {
        if(m_dec == nullptr) {
            return QByteArray();
        }
        QByteArray pcm;
        pcm.resize(MAX_PACKET * sizeof(short) * m_channel);
        int maxFrameCount = MAX_PACKET;
        int frameCount = opus_decode(m_dec, (const uchar*)sample.data(), sample.length(), (opus_int16*)pcm.data(), maxFrameCount, 0);
        if(frameCount > 0) {
            pcm.resize(frameCount*sizeof(short) * m_channel);
            //m_file.write(pcm);
            return pcm;
        }
        return QByteArray();
    }

    QByteArray process(const QByteArray& buf) {
        QDataStream ds(buf);
        qint16 cnt;
        ds >> cnt;
        QByteArray pcm;
        for(int i = 0; i < cnt; i++) {
            QByteArray buf;
            ds >> buf;
            pcm.append(decode(buf));
        }
        return pcm;
    }
};

QKxOpusDecoder::QKxOpusDecoder(int sampleRate, int channel, QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxOpusDecoderPrivate(sampleRate, channel, this);
}

QKxOpusDecoder::~QKxOpusDecoder()
{
    delete m_prv;
}

QByteArray QKxOpusDecoder::process(const QByteArray &buf)
{
    return m_prv->process(buf);
}
