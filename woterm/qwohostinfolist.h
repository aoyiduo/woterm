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

#ifndef QWOHOSTINFILIST_H
#define QWOHOSTINFILIST_H

#include <QDialog>

namespace Ui {
class QWoHostInfoList;
}

class QWoHostInfoList : public QDialog
{
    Q_OBJECT

public:
    explicit QWoHostInfoList(QWidget *parent = nullptr);
    ~QWoHostInfoList();

private:
    Ui::QWoHostInfoList *ui;
};

#endif // QWOHOSTINFILIST_H
