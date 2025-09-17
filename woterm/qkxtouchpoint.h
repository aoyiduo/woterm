/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QKXTOUCHPOINT_H
#define QKXTOUCHPOINT_H

#include <QWidget>
#include <QPointer>
#include <QTouchEvent>

class QMouseEvent;
class QPaintEvent;
class QTimer;

class QKxTouchPoint : public QWidget
{
    Q_OBJECT
public:
    explicit QKxTouchPoint(QWidget *parent = nullptr);
    int raduis() const;
    void setRadius(int w);
    void setOpacity(qreal active, qreal deactive);

signals:
    void clicked();

private slots:
    void onActiveTimeout();
protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual bool event(QEvent *e);
    virtual void touchBeginEvent(QTouchEvent *e);
    virtual void touchUpdateEvent(QTouchEvent *e);
    virtual void touchEndEvent(QTouchEvent *e);
    virtual void touchCancelEvent(QTouchEvent *e);

private:
    int m_radius;
    qreal m_active, m_deactive, m_opacity;
    QPoint m_dragPosition;
    QPointer<QTimer> m_timerActive;    
    QPoint m_ptPressed;
};

#endif // QKXTOUCHPOINT_H
