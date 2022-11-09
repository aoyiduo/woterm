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

#ifndef QWOSFTPEXWIDGET_H
#define QWOSFTPEXWIDGET_H

#include <QWidget>

namespace Ui {
class QWoSftpExWidget;
}

class QWoSftpExWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSftpExWidget(const QString& target, int gid, QWidget *parent = nullptr);
    ~QWoSftpExWidget();

private:
    Ui::QWoSftpExWidget *ui;
};

#endif // QWOSFTPEXWIDGET_H
