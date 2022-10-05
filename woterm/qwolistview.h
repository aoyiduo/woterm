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
