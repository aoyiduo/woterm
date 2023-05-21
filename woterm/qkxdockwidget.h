/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
