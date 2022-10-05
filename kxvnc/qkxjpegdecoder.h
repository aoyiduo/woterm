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

#ifndef QKXJPEGDECODER_H
#define QKXJPEGDECODER_H

#include <QObject>

class QKxJpegDecoderPrivate;
class QKxJpegDecoder : public QObject
{
    Q_OBJECT
public:
    explicit QKxJpegDecoder(QObject *parent = nullptr);
    ~QKxJpegDecoder();
    bool decode(uchar *rgb, int bytesPerLine, int total, uchar *jpg, int cnt);
private:
    QKxJpegDecoderPrivate *m_prv;
};

#endif // QKXJPEGDECODER_H
