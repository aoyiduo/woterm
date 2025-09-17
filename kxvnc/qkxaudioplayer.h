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

#ifndef QKXAUDIOPLAYER_H
#define QKXAUDIOPLAYER_H

#include <QObject>

class QKxAudioPlayerPrivate;
class QKxAudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit QKxAudioPlayer(int sampleRate, int channel, QObject *parent = nullptr);
    virtual ~QKxAudioPlayer();
    bool isRunning();
    bool start();
    void stop();
private slots:
    void onAudioBuffer(const QByteArray & pcm);
private:
    QKxAudioPlayerPrivate *m_prv;
};

#endif // QKXAUDIOPLAYER_H
