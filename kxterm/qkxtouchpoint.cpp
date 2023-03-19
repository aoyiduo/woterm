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

#include "qkxtouchpoint.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>

#define TIMEOUT  (3000)

QKxTouchPoint::QKxTouchPoint(QWidget *parent)
    : QWidget(parent)
    , m_radius(16)
    , m_active(0.7)
    , m_deactive(0.3)
{
    static int idx = 0;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setObjectName(QString("QKxTouchPoint:%1").arg(idx++));
    m_opacity = m_deactive;

    m_timerActive = new QTimer(this);
    QObject::connect(m_timerActive, SIGNAL(timeout()), this, SLOT(onActiveTimeout()));
    setRadius(m_radius);
}

int QKxTouchPoint::raduis() const
{
    return m_radius;
}

void QKxTouchPoint::setRadius(int w)
{
    m_radius = w;
    setFixedSize(m_radius * 2, m_radius * 2);
}

void QKxTouchPoint::setOpacity(qreal active, qreal deactive)
{
    m_active = active;
    m_deactive = deactive;
    m_opacity = deactive;
}

void QKxTouchPoint::onActiveTimeout()
{
    m_timerActive->stop();
    m_opacity = m_deactive;
    update();
}

void QKxTouchPoint::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    QPoint pt(m_radius, m_radius);

    p.setOpacity(m_opacity);

    QPen pen;
    pen.setWidth(3);

    pen.setColor(QColor(Qt::white));
    p.setPen(pen);
    p.setBrush(QColor(Qt::black));
    p.drawEllipse(pt, m_radius - pen.width(), m_radius - pen.width());

    pen.setColor(QColor(Qt::black));
    p.setPen(pen);
    p.setBrush(QColor(Qt::white));
    p.drawEllipse(pt, m_radius / 2, m_radius / 2);
    p.restore();
}

void QKxTouchPoint::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint pt = event->globalPos() - m_dragPosition;
        move(pt);
        event->accept();
    }
    m_timerActive->start(TIMEOUT);
    m_opacity = m_active;
    update();
}

void QKxTouchPoint::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        m_ptPressed = event->globalPos();
    }
    m_timerActive->start(TIMEOUT);
    m_opacity = m_active;
    update();
}

void QKxTouchPoint::mouseReleaseEvent(QMouseEvent *event)
{
    m_timerActive->start(TIMEOUT);
    update();

    QPoint pt = event->globalPos();
    int rw = qAbs(pt.x() - m_ptPressed.x());
    int rh = qAbs(pt.y() - m_ptPressed.y());
    if(rw < 3 && rh < 3) {
        emit clicked();
    }
}

bool QKxTouchPoint::event(QEvent *e)
{
    QEvent::Type type = e->type();
    switch (type) {
    case QEvent::TouchBegin:
        touchBeginEvent(reinterpret_cast<QTouchEvent*>(e));
        if(e->isAccepted()) {
            return true;
        }
        break;
    case QEvent::TouchUpdate:
        touchUpdateEvent(reinterpret_cast<QTouchEvent*>(e));
        if(e->isAccepted()) {
            return true;
        }
        break;
    case QEvent::TouchEnd:
        touchEndEvent(reinterpret_cast<QTouchEvent*>(e));
        if(e->isAccepted()) {
            return true;
        }
        break;
    case QEvent::TouchCancel:
        touchCancelEvent(reinterpret_cast<QTouchEvent*>(e));
        if(e->isAccepted()) {
            return true;
        }
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

void QKxTouchPoint::touchBeginEvent(QTouchEvent *e)
{
    qDebug() << "touchBeginEvent" << (e->target() != nullptr ? e->target()->objectName() : "nullptr") << e->touchPoints();
    e->setAccepted(true);
    const QList<QTouchEvent::TouchPoint> & tps = e->touchPoints();
    if(tps.isEmpty()) {
        return;
    }
    const QPoint& tp = tps.first().screenPos().toPoint();
    m_dragPosition = tp - frameGeometry().topLeft();
    m_ptPressed = tp;

    m_timerActive->start(TIMEOUT);
    m_opacity = m_active;
    update();
}

void QKxTouchPoint::touchUpdateEvent(QTouchEvent *e)
{
    qDebug() << "touchUpdateEvent" << (e->target() != nullptr ? e->target()->objectName() : "nullptr") << e->touchPoints();
    const QList<QTouchEvent::TouchPoint> & tps = e->touchPoints();
    if(tps.isEmpty()) {
        return;
    }
    const QPoint& tp = tps.first().screenPos().toPoint();
    QPoint pt = tp - m_dragPosition;
    move(pt);

    m_timerActive->start(TIMEOUT);
    m_opacity = m_active;
    update();
}

void QKxTouchPoint::touchEndEvent(QTouchEvent *e)
{
    qDebug() << "touchEndEvent" << (e->target() != nullptr ? e->target()->objectName() : "nullptr") << e->touchPoints();

    const QList<QTouchEvent::TouchPoint> & tps = e->touchPoints();
    if(tps.isEmpty()) {
        return;
    }

    m_timerActive->start(TIMEOUT);
    update();

    const QPoint& pt = tps.first().screenPos().toPoint();

    int rw = qAbs(pt.x() - m_ptPressed.x());
    int rh = qAbs(pt.y() - m_ptPressed.y());
    if(rw < 3 && rh < 3) {
        emit clicked();
    }
}

void QKxTouchPoint::touchCancelEvent(QTouchEvent *e)
{
    qDebug() << "touchCancelEvent" << (e->target() != nullptr ? e->target()->objectName() : "nullptr") << e->touchPoints();
}

