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

#include "qkxaudioplayer.h"
#include "qkxresample.h"

#include <portaudio.h>
#include <QDebug>
#include <QMutex>
#include <QPointer>

class QKxAudioPlayerPrivate
{
    QKxAudioPlayer *m_p;
    PaStream* m_stream;
    QMutex m_mtx;
    QByteArray m_pcm;
    int m_channel;
    int m_sampleRate;
    QPointer<QKxResample> m_resample;
public:
    explicit QKxAudioPlayerPrivate(int sampleRate, int channel, QKxAudioPlayer *p)
        : m_p(p){
        m_stream = nullptr;
        m_channel = channel;
        m_sampleRate = sampleRate;
        m_pcm.reserve(1024);
    }

    ~QKxAudioPlayerPrivate() {
        stop();
    }

    static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
        QKxAudioPlayerPrivate *prv = reinterpret_cast<QKxAudioPlayerPrivate*>(userData);
        return prv->onCallBack((char*)outputBuffer, framesPerBuffer, timeInfo, statusFlags);
    }

    int onCallBack(char *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags) {
        QMutexLocker lk(&m_mtx);
        int bytesTotal = framesPerBuffer * m_channel * sizeof(short);
        if(m_pcm.length() > bytesTotal) {
            memcpy(outputBuffer, m_pcm.data(), bytesTotal);
            m_pcm.remove(0, bytesTotal);
        }else{
            int left = bytesTotal - m_pcm.length();
            memcpy(outputBuffer, m_pcm.data(), m_pcm.length());
            char *tmp = outputBuffer + m_pcm.length();
            memset(tmp, 0, left);
            m_pcm.clear();
        }
        return paContinue;
    }

    #define KEEP_SAMPLE_SECOND (1)
    void push(const QByteArray& pcm) {
        QMutexLocker lk(&m_mtx);
        m_pcm.append(pcm);
        int sampleTotal = m_pcm.length() / 2;
        int sampleLeft = m_sampleRate * m_channel * KEEP_SAMPLE_SECOND;
        int countDelete = sampleTotal - sampleLeft;
        if(countDelete > 0) {
            m_pcm.remove(0, countDelete * 2);
        }
    }

    void onAudioBuffer(const QByteArray &pcm) {
        if(m_resample == nullptr) {
            return;
        }
        m_resample->process(pcm.data(), pcm.length());
        QByteArray buf = m_resample->popAll();
        if(!buf.isEmpty()) {
            push(buf);
        }
    }

    bool isRunning() {
        return m_stream != nullptr;
    }

    bool start() {
        if(m_stream != nullptr) {
            return true;
        }
        Pa_Initialize();
        int idx = Pa_GetDefaultOutputDevice();
        if(idx == paNoDevice) {
            return false;
        }
        PaStreamParameters param;
        param.channelCount = m_channel;
        param.device = idx;
        param.sampleFormat = paInt16;
        param.suggestedLatency = Pa_GetDeviceInfo(idx)->defaultLowInputLatency;
        param.hostApiSpecificStreamInfo = nullptr;

        const PaDeviceInfo *di = Pa_GetDeviceInfo(idx);
        if(di == nullptr) {
            return false;
        }

        PaStream* stream;
        int sampleDefault = di->defaultSampleRate;
        int err = Pa_OpenStream(&stream, nullptr, &param, sampleDefault, 2048, paClipOff, playCallback, this);
        if(err != paNoError) {
            qDebug() << "Failed to openStream:" << idx << m_sampleRate << m_channel;
            return false;
        }
        m_pcm.clear();
        if(Pa_StartStream(stream) != paNoError) {
            qDebug() << "failed to start stream:" << idx << m_sampleRate << m_channel;
            Pa_CloseStream(stream);
            return false;
        }
        m_stream = stream;
        m_resample = new QKxResample();
        m_resample->init(m_channel, 48000, 16, m_channel, sampleDefault, 16);
        return true;
    }

    void stop() {
        if(m_resample != nullptr) {
            delete m_resample;
        }

        if(m_stream == nullptr) {
            return;
        }

        Pa_StopStream(m_stream);
        Pa_CloseStream(m_stream);
        Pa_Terminate();
        m_stream = nullptr;
    }
};

QKxAudioPlayer::QKxAudioPlayer(int sampleRate, int channel, QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxAudioPlayerPrivate(sampleRate, channel, this);
}

QKxAudioPlayer::~QKxAudioPlayer()
{
    delete m_prv;
}

bool QKxAudioPlayer::isRunning()
{
    return m_prv->isRunning();
}

bool QKxAudioPlayer::start()
{
    return m_prv->start();
}

void QKxAudioPlayer::stop()
{
    m_prv->stop();
}

void QKxAudioPlayer::onAudioBuffer(const QByteArray &pcm)
{
    m_prv->onAudioBuffer(pcm);
}
