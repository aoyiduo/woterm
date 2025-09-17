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

#include "qwohostinfolist.h"
#include "ui_qwohostinfolist.h"

QWoHostInfoList::QWoHostInfoList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoHostInfoList)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
}

QWoHostInfoList::~QWoHostInfoList()
{
    delete ui;
}
