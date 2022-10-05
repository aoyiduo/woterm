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

#ifndef QWOTERMMASK_H
#define QWOTERMMASK_H

#include "qwowidget.h"

namespace Ui {
class QWoTermMask;
}

class QWoTermMask : public QWoWidget
{
    Q_OBJECT

public:
    explicit QWoTermMask(QWidget *parent = nullptr);
    ~QWoTermMask();

signals:
    void reconnect();

private slots:
    void onReconnect();

private:
    void paintEvent(QPaintEvent* paint);
    void mousePressEvent(QMouseEvent *ev);
    void contextMenuEvent(QContextMenuEvent *ev);
private:
    Ui::QWoTermMask *ui;
};

#endif // QWOTERMMASK_H
