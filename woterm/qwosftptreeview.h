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

#ifndef QWOSFTPTREEVIEW_H
#define QWOSFTPTREEVIEW_H

#include "qwotreeview.h"

#include <QUrl>
#include <QList>

class QWoSftpTreeView : public QWoTreeView
{
    Q_OBJECT
public:
    QWoSftpTreeView(QWidget *parent = nullptr);
    virtual ~QWoSftpTreeView();
signals:
    void dropArrived(const QList<QUrl>& urls);
protected:
    virtual void dragEnterEvent(QDragEnterEvent *ev);
    virtual void dragMoveEvent(QDragMoveEvent *ev);
    virtual void dragLeaveEvent(QDragLeaveEvent* ev);
    virtual void dropEvent(QDropEvent *de);
};

#endif // QWOSFTPTREEVIEW_H
