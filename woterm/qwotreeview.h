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

#ifndef QWOTREEVIEW_H
#define QWOTREEVIEW_H

#include "qwoglobal.h"

#include <QTreeView>

class QWoTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit QWoTreeView(QWidget *parent=nullptr);
signals:
    void itemChanged(const QModelIndex& idx);
    void returnKeyPressed();
private:
    virtual void drawRow(QPainter *painter,
                         const QStyleOptionViewItem &options,
                         const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter,
                              const QRect &rect,
                              const QModelIndex &index) const;
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // QWoTreeView_H
