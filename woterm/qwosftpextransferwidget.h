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

#ifndef QWOSFTPEXTRANSFERWIDGET_H
#define QWOSFTPEXTRANSFERWIDGET_H

#include <QWidget>

namespace Ui {
class QWoSftpExTransferWidget;
}

class QWoSftpExTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSftpExTransferWidget(QWidget *parent = nullptr);
    ~QWoSftpExTransferWidget();

private:
    Ui::QWoSftpExTransferWidget *ui;
};

#endif // QWOSFTPEXTRANSFERWIDGET_H
