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

#ifndef QKXH264DECODER_H
#define QKXH264DECODER_H

#include <QObject>

class QKxH264DecoderPrivate;
class QKxH264Decoder : public QObject
{
public:
    explicit QKxH264Decoder(QObject *parent = nullptr);
    virtual ~QKxH264Decoder();
    bool init(int width, int height);
    int decode(uchar *yuv[3], int *ystride, int *uvstride, uchar *src, int srcLength, bool reset);
private:
    QKxH264DecoderPrivate *m_prv;
};

#endif // QKXH264DECODER_H
