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

#include "qwosftpmovefiledialog.h"
#include "ui_qwosftpmovefiledialog.h"

QWoSftpMoveFileDialog::QWoSftpMoveFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSftpMoveFileDialog)
{
    ui->setupUi(this);
}

QWoSftpMoveFileDialog::~QWoSftpMoveFileDialog()
{
    delete ui;
}
