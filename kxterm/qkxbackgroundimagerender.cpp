/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qkxbackgroundimagerender.h"

#include <QPainter>
#include <QTimer>
#include <QDebug>

QKxBackgroundImageRender::QKxBackgroundImageRender(QObject *parent)
    : QObject(parent)
{
    //QMetaObject::invokeMethod(this, "test", Qt::QueuedConnection);
}

void QKxBackgroundImageRender::setImagePath(const QString &path, const QString &position, bool smooth, int alpha)
{
    QString hash = path + position + QString("%1%2").arg(smooth).arg(alpha);
    if(hash == m_hash) {
        return;
    }
    m_path = path;
    m_smooth = smooth;
    m_alpha = alpha;
    m_tileDirection = renderPosition(position);
    reset();
}

bool QKxBackgroundImageRender::hasImage()
{
    return m_pixmap.isNull();
}

void QKxBackgroundImageRender::render(QPainter *p, const QRect& rt)
{
    ETileDirection tile = m_tileDirection;
    if(tile == ETD_TopLeft) {
        p->drawPixmap(rt.topLeft(), m_pixmap);
    }else if(tile == ETD_TopCenter) {
        QRect drt(0, 0, m_pixmap.width(), m_pixmap.height());
        drt.moveCenter(QPoint(rt.width() / 2, m_pixmap.height() / 2));
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_TopRight) {
        QRect drt(rt.width() - m_pixmap.width(), 0, m_pixmap.width(), m_pixmap.height());
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_TopHorization) {
        QRect drt(0, 0, rt.width(), m_pixmap.height());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_MiddleLeft) {
        QRect drt(0, 0, m_pixmap.width(), m_pixmap.height());
        drt.moveCenter(QPoint(m_pixmap.width()/2, rt.height() / 2));
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_MiddleCenter) {
        QRect drt(0, 0, m_pixmap.width(), m_pixmap.height());
        drt.moveCenter(QPoint(rt.width()/2, rt.height()/2));
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_MiddleRight) {
        QRect drt(0, 0, m_pixmap.width(), m_pixmap.height());
        drt.moveCenter(QPoint(rt.right()-(m_pixmap.width()/2), rt.height()/2));
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_MiddleHorization) {
        QRect drt(0, 0, rt.width(), m_pixmap.height());
        drt.moveCenter(rt.center());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_BottomLeft) {
        QRect drt(0, rt.height() - m_pixmap.height(), m_pixmap.width(), m_pixmap.height());
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_BottomCenter) {
        QRect drt(0, 0, m_pixmap.width(), m_pixmap.height());
        drt.moveCenter(QPoint(rt.width() / 2, rt.height()-m_pixmap.height()/2));
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_BottomRight) {
        QRect drt(rt.width() - m_pixmap.width(), rt.height() - m_pixmap.height(), m_pixmap.width(), m_pixmap.height());
        p->drawPixmap(drt, m_pixmap);
    }else if(tile == ETD_BottomHorization) {
        QRect drt(0, rt.height() - m_pixmap.height(), rt.width(), m_pixmap.height());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_LeftVertical) {
        QRect drt(0, 0, m_pixmap.width(), rt.height());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_MiddleVertical) {
        QRect drt(0, 0, m_pixmap.width(), rt.height());
        drt.moveCenter(rt.center());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_RightVertical) {
        QRect drt(rt.width() - m_pixmap.width(), 0, m_pixmap.width(), rt.height());
        p->drawTiledPixmap(drt, m_pixmap);
    }else if(tile == ETD_Fullscreen) {
        // nothing.
    }
}

QKxBackgroundImageRender::ETileDirection QKxBackgroundImageRender::renderPosition(const QString& pos)
{
    QList<int> dots;
    for(int i = 0; i < pos.length(); i++) {
        if(pos.at(i) == '1') {
            dots.append(i);
        }
    }
    if(dots.isEmpty()) {
        return ETD_None;
    }
    if(dots.length() >= 9) {
        return ETD_Fullscreen;
    }
    if(dots.length() == 1 || dots.length() < 3) {
        int idx = dots.at(0);
        switch (idx) {
        case 0:
            return ETD_TopLeft;
        case 1:
            return ETD_TopCenter;
        case 2:
            return ETD_TopRight;
        case 3:
            return ETD_MiddleLeft;
        case 4:
            return ETD_MiddleCenter;
        case 5:
            return ETD_MiddleRight;
        case 6:
            return ETD_BottomLeft;
        case 7:
            return ETD_BottomCenter;
        case 8:
            return ETD_BottomRight;
        }
        return ETD_None;
    }

    if(dots.at(0) == 0) {
        if(dots.at(1) == 1) {
            return ETD_TopHorization;
        }
        return ETD_LeftVertical;
    }
    if(dots.at(0) == 1) {
        return ETD_MiddleVertical;
    }
    if(dots.at(0) == 2) {
        return ETD_RightVertical;
    }
    if(dots.at(0) == 3) {
        return ETD_MiddleHorization;
    }
    if(dots.at(0) == 6) {
        return ETD_BottomHorization;
    }
    return ETD_None;
}

void QKxBackgroundImageRender::replaceAlpha(QImage &img, int alpha)
{
    if(alpha < 20 || alpha > 230) {
        alpha = 255;
    }
    quint8 *line = img.bits();
    int bytesPerLine = img.bytesPerLine();
    for(int y = 0; y < img.height(); y++, line += bytesPerLine) {
        quint8 *pixel = line;
        for(int x = 0; x < img.width(); x++, pixel += 4) {
            pixel[3] = alpha;
        }
    }
}

void QKxBackgroundImageRender::smoothImage(QImage &img, ETileDirection tile)
{
    if(tile == ETD_TopLeft) {
        smoothImageRight(img);
        smoothImageBottom(img);
    }else if(tile == ETD_TopCenter) {
        smoothImageLeft(img);
        smoothImageRight(img);
        smoothImageBottom(img);
    }else if(tile == ETD_TopRight) {
        smoothImageLeft(img);
        smoothImageBottom(img);
    }else if(tile == ETD_TopHorization) {
        smoothImageBottom(img);
    }else if(tile == ETD_MiddleLeft) {
        smoothImageTop(img);
        smoothImageBottom(img);
        smoothImageRight(img);
    }else if(tile == ETD_MiddleCenter) {
        smoothImageLeft(img);
        smoothImageRight(img);
        smoothImageTop(img);
        smoothImageBottom(img);
    }else if(tile == ETD_MiddleRight) {
        smoothImageLeft(img);
        smoothImageTop(img);
        smoothImageBottom(img);
    }else if(tile == ETD_MiddleHorization) {
        smoothImageTop(img);
        smoothImageBottom(img);
    }else if(tile == ETD_BottomLeft) {
        smoothImageTop(img);
        smoothImageRight(img);
    }else if(tile == ETD_BottomCenter) {
        smoothImageLeft(img);
        smoothImageTop(img);
        smoothImageRight(img);
    }else if(tile == ETD_BottomRight) {
        smoothImageLeft(img);
        smoothImageTop(img);
    }else if(tile == ETD_BottomHorization) {
        smoothImageTop(img);
    }else if(tile == ETD_LeftVertical) {
        smoothImageRight(img);
    }else if(tile == ETD_MiddleVertical) {
        smoothImageLeft(img);
        smoothImageRight(img);
    }else if(tile == ETD_RightVertical) {
        smoothImageLeft(img);
    }else if(tile == ETD_Fullscreen) {
        // nothing.
    }
}

void QKxBackgroundImageRender::smoothImageLeft(QImage &img)
{
    int width = img.width();
    int height = img.height();
    quint8 *rgbptr = img.bits();
    int bytesPerLine = img.bytesPerLine();
    for(int h = 0; h < height; h++) {
        quint8 *line = rgbptr + h * bytesPerLine;
        int alpha = 0;
        for(int w = 0; w < 127 && w < width; w++, alpha+=2) {
            Q_ASSERT(alpha >= 0);
            quint8 *pixel = line + w * 4;
            pixel[3] = pixel[3] * alpha / 255;
        }
    }
}

void QKxBackgroundImageRender::smoothImageRight(QImage &img)
{
    int width = img.width();
    int height = img.height();
    quint8 *rgbptr = img.bits();
    int bytesPerLine = img.bytesPerLine();
    for(int h = 0; h < height; h++) {
        quint8 *line = rgbptr + h * bytesPerLine;
        int alpha = 255;
        for(int w = width - 127; w < width; w++, alpha-=2) {
            Q_ASSERT(alpha >= 0);
            quint8 *pixel = line + w * 4;
            pixel[3] = pixel[3] * alpha / 255;
        }
    }
}

void QKxBackgroundImageRender::smoothImageTop(QImage &img)
{
    int width = img.width();
    int height = img.height();
    quint8 *rgbptr = img.bits();
    int bytesPerLine = img.bytesPerLine();
    int alpha = 0;
    for(int h = 0; h < 127 && h < height; h++, alpha+=2) {
        quint8 *line = rgbptr + h * bytesPerLine;
        for(int w = 0; w < width; w++) {
            Q_ASSERT(alpha >= 0);
            quint8 *pixel = line + w * 4;
            pixel[3] = pixel[3] * alpha / 255;
        }
    }
}

void QKxBackgroundImageRender::smoothImageBottom(QImage &img)
{
    int width = img.width();
    int height = img.height();
    quint8 *rgbptr = img.bits();
    int bytesPerLine = img.bytesPerLine();
    int alpha = 255;
    for(int h = height - 127; h < height; h++, alpha-=2) {
        quint8* line = rgbptr + h * bytesPerLine;
        for(int w = 0; w < width; w++) {
            Q_ASSERT(alpha >= 0);
            quint8 *pixel = line + w * 4;
            pixel[3] = pixel[3] * alpha / 255;
        }
    }
}

void QKxBackgroundImageRender::onTestSwitch()
{
    int tile = m_tileDirection;
    tile++;
    if(tile >= ETD_Fullscreen) {
        tile = ETD_None;
    }
    m_tileDirection = (ETileDirection)tile;
    //m_tileDirection = ETD_TopLeft;
    reset();
    qDebug() << "onTestSwitch" << m_tileDirection;
}

void QKxBackgroundImageRender::test()
{
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTestSwitch()));
    timer->start(5000);
}

void QKxBackgroundImageRender::reset()
{
    m_pixmap = QPixmap();
    QImage img = QImage(m_path);
    if(img.isNull() || img.width() < 128 || img.height() < 128) {
        return;
    }
    QImage image = img.convertToFormat(QImage::Format_ARGB32);
    replaceAlpha(image, m_alpha);
    if(m_smooth) {
        smoothImage(image, m_tileDirection);
    }
    m_pixmap = QPixmap::fromImage(image);
}
