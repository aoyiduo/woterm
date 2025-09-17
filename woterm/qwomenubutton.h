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

#ifndef QWOMENUBUTTON_H
#define QWOMENUBUTTON_H

#include <QWidget>

namespace Ui {
class QWoMenuButton;
}

class QWoMenuButton : public QWidget
{
    Q_OBJECT

public:
    explicit QWoMenuButton(QWidget *parent = nullptr);
    ~QWoMenuButton();

private:
    Ui::QWoMenuButton *ui;
};

#endif // QWOMENUBUTTON_H
