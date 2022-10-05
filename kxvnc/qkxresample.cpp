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

#include "qkxresample.h"
#include "resample/libresample.h"

inline float clip_pcm_float(float v)
{
	if(v > 1.0)
	{
		v = 1.0;
	}
	else if(v < -1.0)
	{
		v = -1.0;
	}
	return v;
}

inline short clip_pcm_short(int v)
{
	if(v > 0x7FFF)
	{
		v = 0x7FFF;
	}
	else if(v < -0x7FFF)
	{
		v = -0x7FFF;
	}
	return v;
}

QKxResample::QKxResample(QObject *parent)
    : QObject(parent)
{

}

QKxResample::~QKxResample()
{
    stopResample();
}

void QKxResample::init( int dsb_channel, int dsb_sampleRate, int dsb_sampleBits, int dst_channel, int dst_sampleRate, int dst_sampleBits)
{
    m_dsb_channel = dsb_channel;
    m_dsb_sampleBits = dsb_sampleBits;
    m_dsb_sampleRate = dsb_sampleRate;

    m_dst_channel = dst_channel;
    m_dst_sampleBits = dst_sampleBits;
    m_dst_sampleRate = dst_sampleRate;

    startResample(m_dsb_sampleRate, m_dst_sampleRate);
}

void QKxResample::process(const char *pcm, int cnt)
{
    runResample(pcm, cnt);
}

void QKxResample::runResample(const char *buf, int length )
{
	if(buf == NULL || length <= 0)
		return;

	if(!(m_dsb_sampleBits == 8 || m_dsb_sampleBits == 16 || m_dsb_sampleBits == 32))
		return;

	double ratio = (double)m_dst_sampleRate / (double)m_dsb_sampleRate;
	m_resample_srcpcm.append(buf, length);

	int nSamplePerChannel = m_resample_srcpcm.size() * 8 / m_dsb_sampleBits / m_dsb_channel;
	int nResampleChannel = m_dsb_channel >= 2 ? 2 : 1;
	int srcChnBytes = nSamplePerChannel * sizeof(float);
	int dstChnBytes = (nSamplePerChannel * ratio + 1024) * sizeof(float);
	if(m_resample_srcchn[0].capacity() < srcChnBytes)
	{
		m_resample_srcchn[0].reserve(srcChnBytes);
		m_resample_srcchn[1].reserve(srcChnBytes);
		m_resample_dstchn[0].reserve(dstChnBytes);
		m_resample_dstchn[1].reserve(dstChnBytes);
	}
	unsigned char *pSample = (unsigned char *)m_resample_srcpcm.c_str();
	int inUsedRef = 0;
	int outRef = 0;
	for(int i = 0; i < m_dsb_channel && i < 2; i++)
	{
		float *src = (float *)m_resample_srcchn[i].c_str();
		float *dst = (float *)m_resample_dstchn[i].c_str();
		for(int j = 0; j < nSamplePerChannel; j++)
		{
			float v = 0.0;
			if(m_dsb_sampleBits == 8)
			{
				unsigned char* pSample8 = pSample;
				v = pSample8[j * m_dsb_channel + i];
				v -= 128.0;
				v /= 128.0;
			}
			else if(m_dsb_sampleBits == 16)
			{
				short *pSample16 = (short*)pSample;
				v = pSample16[j * m_dsb_channel + i];
				v /= 0x7FFF;
			}
			else
			{
				float *pSample32 = (float*)pSample;
				v = pSample32[j * m_dsb_channel + i];
			}
			src[j] = v;
		}
		int inUsed = 0;
		int out = resample_process(m_hResample[i], ratio, src, nSamplePerChannel, false, &inUsed, dst, nSamplePerChannel+1024);
		if(i == 0)
		{
			inUsedRef = inUsed;
			outRef = out;
		}
		else
		{
			if (inUsedRef != inUsed || outRef != out) 
			{
				//fprintf(stderr, "Fatal error: channels out of sync!\n");
				//exit(-1);
				return;
			}
		}
	}

    // remove the data which had been done.
	int usedCount = inUsedRef * m_dsb_channel * m_dsb_sampleBits / 8;
	m_resample_srcpcm.erase(0, usedCount);

    // write data.
	for(int iSample = 0; iSample < outRef; iSample++)
	{
		for(int iChannel = 0; iChannel < nResampleChannel; iChannel++)
		{
			float *dstchn = (float*)m_resample_dstchn[iChannel].c_str();
			float v = dstchn[iSample];
			if(v > 1.0)
			{
				v = 1.0;
			}
			else if(v < -1.0)
			{
				v = -1.0;
			}

			if(m_dst_sampleBits == 8)
			{
				int isample = (int)(v * 128 + 128);
				if(isample > 0xFF)
				{
					isample = 0xFF;
				}
				else if(isample < 0)
				{
					isample = 0;
				}
				unsigned char sample = (unsigned char)isample;
				m_resample_dstpcm.append((char*)&sample, sizeof(unsigned char));
			}
			else if(m_dst_sampleBits == 16)
			{
				int isample = (int)(v * 0x7FFF);
				if(isample > 0x7FFF)
				{
					isample = 0x7FFF;
				}
				else if(isample < -0x7FFF)
				{
					isample = -0x7FFF;
				}
				short sample = (short)isample;
				m_resample_dstpcm.append((char*)&sample, sizeof(short));
			}
			else
			{
				float sample = v;
				m_resample_dstpcm.append((char*)&sample, sizeof(float));
			}
		}
	}

	int sampleBytes = m_dst_sampleBits / 8;
	int nsample = m_resample_dstpcm.length() / sampleBytes;
	int nUsed = writeSamples(m_resample_dstpcm.c_str(), nsample, nResampleChannel, m_dst_sampleBits);
	m_resample_dstpcm.erase(0, nUsed * sampleBytes);
}

void QKxResample::startResample( int srcSampleRate, int dstSampleRate )
{
	double ratio = (double)dstSampleRate / (double)srcSampleRate;
	m_resample_srcpcm.clear();
	m_resample_dstpcm.clear();
	m_hResample[0] = resample_open(1, ratio, ratio);
	m_hResample[1] = resample_open(1, ratio, ratio);
}

void QKxResample::stopResample( )
{
	resample_close(m_hResample[0]);
	resample_close(m_hResample[1]);
}

int QKxResample::writeSamples( const char *buf, int nSample, int nChannel, int nSampleBit )
{
	if(m_dst_sampleBits == 8)
	{
        // ignore 8bit data.
		return nSample;
	}


	if(nChannel == m_dst_channel)
	{
		pushSamples(buf, nSample);
		return nSample;
	}
	
	m_cache_dst.clear();
	if(m_dst_channel == 1)
	{
        // combine two channel.
		if(m_dst_sampleBits == 16)
		{
			short *pSample = (short*)buf;
			for(int i = 0; i < nSample; i+=2)
			{
				int vleft = pSample[i];
				int vright = pSample[i+1];
				short v = (short)((vleft + vright) / 2);
				m_cache_dst.append((char*)&v, sizeof(short));
			}
		}
		else
		{
			float *pSample = (float*)buf;
			for(int i = 0; i < nSample; i+=2)
			{
				float vleft = pSample[i];
				float vright = pSample[i+1];
				float v = (vleft + vright) / 2;
				m_cache_dst.append((char*)&v, sizeof(float));
			}
		}
		pushSamples((char*)m_cache_dst.c_str(), nSample / 2);
		return nSample;
	}

    /* Mono to dual */
	if(m_dst_sampleBits == 16)
	{
		short *pSample = (short*)buf;
		for(int i = 0; i < nSample; i++)
		{
			short v = pSample[i];
			m_cache_dst.append((char*)&v, sizeof(short));
			m_cache_dst.append((char*)&v, sizeof(short));
		}
	}
	else
	{
		float *pSample = (float*)buf;
		for(int i = 0; i < nSample; i++)
		{
			float v = pSample[i];
			m_cache_dst.append((char*)&v, sizeof(float));
			m_cache_dst.append((char*)&v, sizeof(float));
		}
	}
	pushSamples((char*)m_cache_dst.c_str(), nSample * 2);
	return nSample;
}

/*
 * When saving samples, you can save as much as you like. At most, you can only save the data in the last 1 second
*/
#define KEEP_SAMPLE_SECOND (1)
void QKxResample::pushSamples( const char* buf, int nsample )
{
	int nBytePerSample = m_dst_sampleBits / 8;
	m_output_buffer.append(buf, nsample * nBytePerSample);
	int nSamplesAll = m_output_buffer.length() / nBytePerSample;
	int nSamplesSec = m_dst_sampleRate * m_dst_channel * KEEP_SAMPLE_SECOND;
	int nSamplesErase = nSamplesAll - nSamplesSec;
	if(nSamplesErase > 0)
	{
		m_output_buffer.erase(0, nSamplesErase * nBytePerSample);
	}
}

int QKxResample::popSamples( char *buf, int nsample )
{
	int nBytePerSample = m_dst_sampleBits / 8;
	int nSamplesAll = m_output_buffer.length() / nBytePerSample;

	if(nSamplesAll < nsample)
		return 0;

	int length = nsample * nBytePerSample;
	memcpy(buf, m_output_buffer.c_str(), length);
	m_output_buffer.erase(0, length);
    return nsample;
}

QByteArray QKxResample::popAll()
{
    QByteArray out(m_output_buffer.data(), m_output_buffer.length());
    m_output_buffer.clear();
    return out;
}
