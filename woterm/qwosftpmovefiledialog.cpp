/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
