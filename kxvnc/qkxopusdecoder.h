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
