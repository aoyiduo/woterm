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

#include "qkxloadingbarwidget.h"

#include <QTimer>
#include <QPainter>

QKxLoadingBarWidget::QKxLoadingBarWidget(const QColor& clr, QWidget *parent)
    : QWidget(parent)
    , m_color(clr)
    , m_msgColor(clr)
    , m_angle(0)
    , m_step(30)
    , m_txtSize(20)
    , m_thickness(0)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(33);
    m_timer.reset(timer);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void QKxLoadingBarWidget::setSpeed(int fps)
{
    m_timer->setInterval(1000 / fps);
}

void QKxLoadingBarWidget::setThickness(int n)
{
    m_thickness = n;
}

void QKxLoadingBarWidget::setMessageColor(const QColor &clr)
{
    m_msgColor = clr;
}

void QKxLoadingBarWidget::setMessageSize(int sz)
{
    m_txtSize = sz;
}

void QKxLoadingBarWidget::setMessage(const QString &msg)
{
    m_message = msg;
}

void QKxLoadingBarWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath ring;
    QRectF dest = rect();
    if(m_thickness > 0) {
        QPointF pt = dest.center();
        dest.setHeight(m_thickness);
        dest.moveCenter(pt);
    }
    ring.addRect(dest);
    QLinearGradient cg(dest.left(), dest.top(), dest.right(), dest.top());
    cg.setColorAt(0, QColor(Qt::transparent));
    qreal x = m_angle / 360;
    cg.setColorAt(x, QColor(m_color));
    cg.setColorAt(1, QColor(Qt::transparent));
    p.setBrush(cg);
    p.setPen(QColor::fromRgba(0));
    p.drawPath(ring);

    if(!m_message.isEmpty()) {
        QFont ft = p.font();
        ft.setPixelSize(m_txtSize);
        ft.setBold(true);
        p.setFont(ft);
        QPen pen = p.pen();
        pen.setColor(m_msgColor);
        p.setPen(pen);
        p.drawText(rect(), Qt::AlignCenter, m_message);
    }
}

void QKxLoadingBarWidget::showEvent(QShowEvent *ev)
{
    m_timer->start();
    m_angle = 0;
}

void QKxLoadingBarWidget::hideEvent(QHideEvent *ev)
{
    m_timer->stop();
}

void QKxLoadingBarWidget::onTimeout()
{
    m_angle += m_step;
    if(m_angle > 350) {
        m_angle = 350;
        m_step = -m_step;
    }else if(m_angle < 10) {
        m_angle = 10;
        m_step = -m_step;
    }
    update(rect());
}
