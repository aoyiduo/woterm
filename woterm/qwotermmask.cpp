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

#include "qwotermmask.h"
#include "ui_qwotermmask.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

QWoTermMask::QWoTermMask(QWidget *parent) :
    QWoWidget(parent),
    ui(new Ui::QWoTermMask)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setVisible(false);
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnReconnect, SIGNAL(clicked()), this, SLOT(onReconnect()));
}

QWoTermMask::~QWoTermMask()
{
    delete ui;
}

void QWoTermMask::onReconnect()
{
    hide();
    emit reconnect();
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
}
