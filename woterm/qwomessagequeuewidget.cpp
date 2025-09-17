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

#include "qwomessagequeuewidget.h"
#include "ui_qwomessagequeuewidget.h"
#include "qwomessagequeuemodel.h"

QWoMessageQueueWidget::QWoMessageQueueWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QWoMessageQueueWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);
    m_model = new QWoMessageQueueModel(this);
    ui->queue->setModel(m_model);
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QWoMessageQueueWidget::~QWoMessageQueueWidget()
{
    delete ui;
}

void QWoMessageQueueWidget::warning(const QString &title, const QString &content)
{
    m_model->warning(title, content);
    if(!isVisible()) {
        show();
        raise();
    }
}

void QWoMessageQueueWidget::init()
{
    for(int i = 0; i < 30; i++) {
        warning(QString("title:%1").arg(i), QString("content:%1").arg(i));
    }
}
