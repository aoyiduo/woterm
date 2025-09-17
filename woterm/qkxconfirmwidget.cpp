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
#include "qkxconfirmwidget.h"
#include "ui_qkxconfirmwidget.h"

QKxConfirmWidget::QKxConfirmWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QKxConfirmWidget)
{
    ui->setupUi(this);
}

QKxConfirmWidget::~QKxConfirmWidget()
{
    delete ui;
}
