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
