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

#ifndef QWOLISTVIEW_H
#define QWOLISTVIEW_H

#include <QListView>

class QWoListView : public QListView
{
    Q_OBJECT
public:
    explicit QWoListView(QWidget *parent=nullptr);
signals:
    void itemChanged(const QModelIndex& idx);
    void returnKeyPressed();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // QWOLISTVIEW_H
