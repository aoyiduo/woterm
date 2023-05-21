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

#ifndef QKXBACKGROUNDIMAGERENDER_H
#define QKXBACKGROUNDIMAGERENDER_H

#include "qkxterm_share.h"

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QList>

class QKxTermItem;

class QTERM_EXPORT QKxBackgroundImageRender : public QObject
{
    Q_OBJECT
public:
    enum ETileDirection {
        ETD_None = 0,
        ETD_TopLeft,
        ETD_TopCenter,
        ETD_TopRight,
        ETD_TopHorization,
        ETD_MiddleLeft,
        ETD_MiddleCenter,
        ETD_MiddleRight,
        ETD_MiddleHorization,
        ETD_BottomLeft,
        ETD_BottomCenter,
        ETD_BottomRight,
        ETD_BottomHorization,
        ETD_LeftVertical,
        ETD_MiddleVertical,
        ETD_RightVertical,
        ETD_Fullscreen
    };
    Q_ENUM(ETileDirection);
public:
    explicit QKxBackgroundImageRender(QObject *parent = nullptr);
    void setImagePath(const QString& path, const QString& position, bool smooth, int alpha);
    bool hasImage();
protected:
    void render(QPainter *p, const QRect& rt);
private:
    ETileDirection renderPosition(const QString& pos);
    void replaceAlpha(QImage &img, int alpha);
    void smoothImage(QImage &img, ETileDirection tile);
    void smoothImageLeft(QImage &img);
    void smoothImageRight(QImage &img);
    void smoothImageTop(QImage &img);
    void smoothImageBottom(QImage &img);
private slots:
    void onTestSwitch();
private:
    Q_INVOKABLE void test();
    void reset();
private:    
    QString m_path, m_hash;
    QPixmap m_pixmap;
    ETileDirection m_tileDirection;
    bool m_smooth;
    int m_alpha;
    friend class QKxTermItem;
};

#endif // QKXBACKGROUNDIMAGERENDER_H
