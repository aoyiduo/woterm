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
