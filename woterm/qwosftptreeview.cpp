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

#include "qwosftptreeview.h"

#include <QMimeData>
#include <QFile>
#include <QDragEnterEvent>
#include <QFileInfo>

QWoSftpTreeView::QWoSftpTreeView(QWidget *parent)
    : QWoTreeView(parent)
{

}

QWoSftpTreeView::~QWoSftpTreeView()
{

}

void QWoSftpTreeView::dragEnterEvent(QDragEnterEvent *de)
{
    const QMimeData *md = de->mimeData();
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        for(auto it = urls.begin(); it != urls.end(); it++) {
            const QUrl& url = *it;
            QString localFile = url.toLocalFile();
            QFileInfo fi(localFile);
            if(fi.exists()) {
                de->setDropAction(Qt::MoveAction);
                de->setAccepted(true);
                return;
            }
        }
    }else{
        de->ignore();
    }
}

void QWoSftpTreeView::dragMoveEvent(QDragMoveEvent *ev)
{
    ev->acceptProposedAction();
    ev->accept();
}

void QWoSftpTreeView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    QTreeView::dragLeaveEvent(ev);
}

void QWoSftpTreeView::dropEvent(QDropEvent *de)
{
    const QMimeData *md = de->mimeData();
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        emit dropArrived(urls);
    }else{
        de->ignore();
    }
}
