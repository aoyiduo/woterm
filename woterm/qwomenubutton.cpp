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

#include "qwomenubutton.h"
#include "ui_qwomenubutton.h"

QWoMenuButton::QWoMenuButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QWoMenuButton)
{
    ui->setupUi(this);
    ui->btnDefault->setObjectName("btnGroupDefault");
    ui->btnMenu->setObjectName("btnGroupMenu");
}

QWoMenuButton::~QWoMenuButton()
{
    delete ui;
}
