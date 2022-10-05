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

#include "qkxjpegdecoder.h"

#include <turbojpeg.h>

class QKxJpegDecoderPrivate
{
public:
    QKxJpegDecoderPrivate(QKxJpegDecoder *p) {

    }

    ~QKxJpegDecoderPrivate() {

    }

    bool decode(uchar *rgb, int bytesPerLine, int total, uchar *jpg, int cnt) {
        int width = 0, height = 0, jpegsubsamp = 0;
        void* decompressor = tjInitDecompress();
        if (nullptr == decompressor) {
            return false;
        }

        if (tjDecompressHeader2(decompressor, jpg, cnt, &width, &height, &jpegsubsamp) != 0) {
            tjDestroy(decompressor);
            return false;
        }

        if(total < height * bytesPerLine) {
            return false;
        }
        int pitch = tjPixelSize[TJPF::TJPF_BGRX] * width;
        int flags = TJFLAG_FASTDCT;
        int status = tjDecompress2(decompressor, jpg, cnt, rgb, width, bytesPerLine, height, TJPF::TJPF_BGRX, flags);
        if ( status != 0) {
            tjDestroy(decompressor);
            return false;
        }

        tjDestroy(decompressor);
        return true;
    }

    bool decompress(uint8_t* data_compressed, uint32_t size, uint8_t* &out_uncompressed, int pixel_format)
    {
        if (nullptr == data_compressed || nullptr == out_uncompressed) {
            return false;
        }

        int width = 0, height = 0, jpegsubsamp = 0;
        void* decompressor = tjInitDecompress();
        if (nullptr == decompressor) {
            return false;
        }

        if (tjDecompressHeader2(decompressor, data_compressed, size, &width, &height, &jpegsubsamp) != 0) {
            tjDestroy(decompressor);
            return false;
        }

        // pixel_format : TJPF::TJPF_BGR or other
        int pitch = tjPixelSize[pixel_format] * width;
        uint32_t len = pitch * height;
        int flags = TJFLAG_FASTDCT;
        int status = tjDecompress2(decompressor, data_compressed, len, out_uncompressed, width, pitch,
            height, pixel_format, flags);
        if ( status != 0) {
            tjDestroy(decompressor);
            return false;
        }

        tjDestroy(decompressor);
        return true;
    }
};

QKxJpegDecoder::QKxJpegDecoder(QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxJpegDecoderPrivate(this);
}

QKxJpegDecoder::~QKxJpegDecoder()
{
    delete m_prv;
}

bool QKxJpegDecoder::decode(uchar *rgb, int bytesPerLine, int total, uchar *jpg, int cnt)
{
    return m_prv->decode(rgb, bytesPerLine, total, jpg, cnt);
}
