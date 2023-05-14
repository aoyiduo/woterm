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
