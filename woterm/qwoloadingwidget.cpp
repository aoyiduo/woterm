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

#include "qwoloadingwidget.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

QWoLoadingWidget::QWoLoadingWidget(const QColor& clr, QWidget *parent)
    : QWidget(parent)
    , m_color(clr)
{
    m_timer.setInterval(33);
    m_angle = 0;
    m_radius = 20;
    setVisible(false);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(onRotateRequest()));
}

QWoLoadingWidget::QWoLoadingWidget(const QString &imagePath, QWidget *parent)
    : QWidget(parent)
{
    m_timer.setInterval(33);
    m_angle = 0;
    m_radius = 20;
    m_pixmap = QPixmap(imagePath);
    setVisible(false);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(onRotateRequest()));

    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

QWoLoadingWidget::~QWoLoadingWidget()
{

}

void QWoLoadingWidget::setSpeed(int fps)
{
    m_timer.setInterval(1000 / fps);
}

void QWoLoadingWidget::setRadius(int n)
{
    m_radius = n;
}

void QWoLoadingWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if(m_pixmap.isNull()) {
        //draw loading ring
        QPainterPath ring;
        QRectF dest = QRectF(width() / 2 - m_radius,
                             height() / 2 - m_radius,
                             m_radius * 2,
                             m_radius * 2);
        QPointF center = dest.center();
        qreal radiusSup = qMin(dest.width(), dest.height()) / 2;
        qreal outterRadius = radiusSup;
        qreal innerRadius = radiusSup * (1 - 0.3);
        ring.addEllipse(center, outterRadius, outterRadius);
        ring.addEllipse(center, innerRadius, innerRadius);
        ring.setFillRule(Qt::OddEvenFill);
        QConicalGradient cg(center, -m_angle);

        //simulate gamma correction by 5 stops, gamma = 2.2
        QColor c0(m_color);
        QColor c1(m_color);
        QColor c2(m_color);
        QColor c3(m_color);
        QColor c4(m_color);
        QColor c5(m_color);
        c0.setAlpha(0xff);
        c1.setAlpha(0xf7);
        c2.setAlpha(0xe3);
        c3.setAlpha(0xb0);
        c4.setAlpha(0x5a);
        c5.setAlpha(0x1f);
        cg.setColorAt(0.0, c0);
        cg.setColorAt(0.2, c1);
        cg.setColorAt(0.4, c2);
        cg.setColorAt(0.6, c3);
        cg.setColorAt(0.8, c4);
        cg.setColorAt(1.0, c5);
        p.setBrush(cg);
        p.setPen(QColor::fromRgba(0));
        p.drawPath(ring);
    }else{
        QRect winRt = rect();
        QRect drawRt(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
        p.translate(winRt.center());
        p.rotate(m_angle);
        p.drawPixmap(drawRt, m_pixmap);
        p.drawRect(drawRt);
    }
}

void QWoLoadingWidget::showEvent(QShowEvent *ev)
{
    m_timer.start();
    m_angle = 0;
}

void QWoLoadingWidget::hideEvent(QHideEvent *ev)
{
    m_timer.stop();
}

void QWoLoadingWidget::resizeEvent(QResizeEvent *ev)
{
    QRect rt = QRect(width() / 2 - m_radius,
                         height() / 2 - m_radius,
                         m_radius * 2,
                         m_radius * 2);
    QRegion rgn(rt.left(), rt.top(), rt.width(), rt.height());
    setMask(rgn);
}

void QWoLoadingWidget::onRotateRequest()
{
    m_angle += 30;
    if(m_angle > 360) {
        m_angle -= 360;
    }
    QRect winRt = rect();
    QRect drawRt(0, 0, m_radius * 2, m_radius * 2);
    drawRt.adjust(-2,-2,2,2);
    drawRt.moveCenter(winRt.center());
    update(drawRt);
}
