/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QKXDOCKWIDGET_H
#define QKXDOCKWIDGET_H

#include <QDockWidget>

class QKxDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit QKxDockWidget(const QString &title, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // QKXDOCKWIDGET_H
