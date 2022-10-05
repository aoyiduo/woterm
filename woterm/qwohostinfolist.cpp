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
