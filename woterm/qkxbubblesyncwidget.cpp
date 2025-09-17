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

#include "qkxbubblesyncwidget.h"
#include "ui_qkxbubblesyncwidget.h"

#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QCursor>
#include <QDebug>

QKxBubbleSyncWidget::QKxBubbleSyncWidget() :
    QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint | Qt::ToolTip),
    ui(new Ui::QKxBubbleSyncWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    setAttribute(Qt::WA_StyledBackground);

    ui->content->setReadOnly(true);

    QString style = "QToolButton{border:0;border-radius:3px;}\r\n";
    style += "QToolButton:hover{background-color:rgba(255,255,255,128)}\r\n";
    style += "QToolButton:pressed{background-color:rgba(200,200,200,128)}";
    ui->btnClose->setStyleSheet(style);

    setFixedSize(240, 180);
    resetLayout();
}

QKxBubbleSyncWidget::~QKxBubbleSyncWidget()
{
    delete ui;
}

void QKxBubbleSyncWidget::setMessage(const QString& title, const QString &msg, int timeout)
{
    Task t;
    t.content = msg;
    t.title = title;
    t.timeout = timeout;
    m_tasks.append(t);
    if(m_tasks.length() > 1) {
        return;
    }
    if(m_timer == nullptr) {
        m_timer = new QTimer(this);
        QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }
    m_timer->start(timeout);
    ui->title->setText(title);
    ui->content->setPlainText(msg);
    resetLayout();
}

void QKxBubbleSyncWidget::onResetLayout()
{
    QSize sz = size();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect rt = screen->geometry();
    rt.setLeft(rt.right() - sz.width());
    rt.setTop(rt.bottom() - sz.height());
    setGeometry(rt);
    show();
}

void QKxBubbleSyncWidget::onTimeout()
{
    if(m_tasks.isEmpty()) {
        if(isCursorHover()) {
            return;
        }
        deleteLater();
        return;
    }
    m_tasks.takeFirst();
    if(m_tasks.isEmpty()) {
        if(isCursorHover()) {
            return;
        }
        deleteLater();
        return;
    }
    Task t = m_tasks.first();
    ui->title->setText(t.title);
    ui->content->setPlainText(t.content);
    m_timer->start(t.timeout);
}

void QKxBubbleSyncWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void QKxBubbleSyncWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

bool QKxBubbleSyncWidget::isCursorHover()
{
    QPoint pt = QCursor::pos();
    QRect rt = geometry();
    //qDebug() << pt << rt << rt.contains(pt);
    return rt.contains(pt);
}

void QKxBubbleSyncWidget::resetLayout()
{
    QMetaObject::invokeMethod(this, "onResetLayout", Qt::QueuedConnection);
}
