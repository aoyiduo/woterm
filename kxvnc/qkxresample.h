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

#ifndef QKXRESAMPLE_H
#define QKXRESAMPLE_H

#include <string>
#include <QByteArray>
#include <QObject>

class QKxResample : public QObject
{
public:
    explicit QKxResample(QObject *parent = nullptr);
    ~QKxResample();
    void init(int dsb_channel, int dsb_sampleRate, int dsb_sampleBits, int dst_channel, int dst_sampleRate, int dst_sampleBits);
    void process(const char* pcm, int cnt);
    int popSamples(char *buf, int nsample);
    QByteArray popAll();
private:
    /*The principle of algorithmic implementation is: the number of data pushed here is the number of data extracted.*/
    void pushSamples(const char* buf, int nsample);

	void startResample(int srcSampleRate, int dstSampleRate);
    void runResample(const char *buf, int length);
	int writeSamples(const char *buf, int nSample, int nChannel, int nSampleBit);
	void stopResample();
private:
    int m_dsb_channel;// Collected information.
	int m_dsb_sampleRate;
	int m_dsb_sampleBits;//8,16,32

    int m_dst_channel;// The number of target audio to be converted.
	int m_dst_sampleRate;
	int m_dst_sampleBits;

    std::string m_output_buffer;// Store audio data conforming to DST requirements.

	std::string m_cache_src;
	std::string m_cache_dst;

    /*resample*/
    std::string m_resample_srcpcm; // The collected data has not been converted.
    std::string m_resample_dstpcm; // The converted OK.
    std::string m_resample_srcchn[2];// Pre conversion temporary buffer for resampling.
    std::string m_resample_dstchn[2];// Temporary buffer for receiving color samples.
	void *m_hResample[2];
};

#endif
