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

#ifndef QKXOPUSDECODER_H
#define QKXOPUSDECODER_H

#include <QObject>

class QKxOpusDecoderPrivate;
class QKxOpusDecoder : public QObject
{
    Q_OBJECT
public:
    explicit QKxOpusDecoder(int sampleRate, int channel, QObject *parent = nullptr);
    virtual ~QKxOpusDecoder();
    QByteArray process(const QByteArray & buf);
private:
    QKxOpusDecoderPrivate *m_prv;
};

#endif // QKXOPUSDECODER_H
