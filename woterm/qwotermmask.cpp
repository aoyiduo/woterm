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

#include "qwotermmask.h"
#include "ui_qwotermmask.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

#define MAX_RECONNECT_WAIT_COUNT        (10)

QWoTermMask::QWoTermMask(QWidget *parent)
    : QWoWidget(parent)
    , ui(new Ui::QWoTermMask)
    , m_autoReconnect(false)
    , m_reconnectWaitCount(MAX_RECONNECT_WAIT_COUNT)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setVisible(false);
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnReconnect, SIGNAL(clicked()), this, SLOT(onReconnect()));
    m_connectLabel = ui->btnReconnect->text();
    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

QWoTermMask::~QWoTermMask()
{
    delete ui;
}

void QWoTermMask::setAutoReconnect(bool on)
{
    m_autoReconnect = on;

}

void QWoTermMask::onReconnect()
{
    hide();
    emit reconnect();
}

void QWoTermMask::onTimeout()
{
    m_reconnectWaitCount--;
    if(m_reconnectWaitCount < 0) {
        m_timer->stop();
        ui->btnReconnect->setText(m_connectLabel);
        onReconnect();
        return;
    }
    QString label = m_connectLabel + QString("(%1)").arg(m_reconnectWaitCount);
    ui->btnReconnect->setText(label);
}

void QWoTermMask::mousePressEvent(QMouseEvent *ev)
{
    ev->accept();
}

void QWoTermMask::contextMenuEvent(QContextMenuEvent *ev)
{
    return;
}

void QWoTermMask::showEvent(QShowEvent *ev)
{
    QWoWidget::showEvent(ev);
    raise();

    if(m_timer) {
        if(m_autoReconnect) {
            m_reconnectWaitCount = MAX_RECONNECT_WAIT_COUNT;
            m_timer->start(1000);
        }else{
            m_timer->stop();
        }
    }
}

void QWoTermMask::hideEvent(QHideEvent *ev)
{
    QWoWidget::hideEvent(ev);
    m_timer->stop();
}
