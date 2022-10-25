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
#ifndef QKXCONFIRMWIDGET_H
#define QKXCONFIRMWIDGET_H

#include <QWidget>

namespace Ui {
class QKxConfirmWidget;
}

class QKxConfirmWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QKxConfirmWidget(QWidget *parent = nullptr);
    ~QKxConfirmWidget();

private:
    Ui::QKxConfirmWidget *ui;
};

#endif // QKXCONFIRMWIDGET_H
