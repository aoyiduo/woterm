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
