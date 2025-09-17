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
