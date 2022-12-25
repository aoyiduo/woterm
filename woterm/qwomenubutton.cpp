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
