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

#pragma once

#include <QWidget>

#include <QPointer>

class QWoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWoWidget(QWidget *parent=nullptr);
signals:
    void aboutToClose(QCloseEvent* event);
private:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *e);
};
