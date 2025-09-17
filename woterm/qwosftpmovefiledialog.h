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

#ifndef QWOSFTPMOVEFILEDIALOG_H
#define QWOSFTPMOVEFILEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSftpMoveFileDialog;
}

class QWoSftpMoveFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSftpMoveFileDialog(QWidget *parent = nullptr);
    ~QWoSftpMoveFileDialog();

private:
    Ui::QWoSftpMoveFileDialog *ui;
};

#endif // QWOSFTPMOVEFILEDIALOG_H
