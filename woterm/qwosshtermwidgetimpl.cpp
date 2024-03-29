﻿/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwosshtermwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwosshtermwidget.h"
#include "qwosftpwidget.h"
#include "qwomainwindow.h"
#include "qkxver.h"

#include <QTimer>
#include <QMenu>


QWoSshTermWidgetImpl::QWoSshTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoTermWidgetImpl(target, gid, tab, parent)
{

}

QWoSshTermWidgetImpl::~QWoSshTermWidgetImpl()
{

}

QWoTermWidget *QWoSshTermWidgetImpl::createTermWidget(const QString &target, int gid, QWidget *parent)
{
    QWoSshTermWidget *w = new QWoSshTermWidget(target, gid, parent);
    QString name = tr("%1:%2").arg(increaseId()).arg(target);
    w->setTermName(name);
    w->setObjectName(name);
    w->setImplementWidget(this);

    QObject::connect(w, SIGNAL(activePathArrived(QString)), this, SLOT(onActivePathArrived(QString)));
    QObject::connect(w, SIGNAL(sftpAssistant()), this, SLOT(onSftpAssistOpen()));
    return w;
}

void QWoSshTermWidgetImpl::handleTabContextMenu(QMenu *menu)
{
    QWoTermWidgetImpl::handleTabContextMenu(menu);
    if(m_sftp == nullptr) {
        menu->addAction(tr("Open sftp assistant"), this, SLOT(onSftpAssistOpen()));
    }else{
        menu->addAction(tr("Close sftp assistant"), this, SLOT(onSftpAssistOpen()));
    }
    menu->addAction(tr("Multiplexing sftp sessions"), this, SLOT(onNewSftpSession()));
    menu->addAction(tr("Multiplexing ssh sessions"), this, SLOT(onNewSshSession()));
}

QWoShowerWidget::ESessionState QWoSshTermWidgetImpl::sessionState()
{
    QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->isConnected()) {
        return eDisconnected;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(*it);
        if(term == nullptr || !term->isConnected()) {
            return eOtherDisconnected;
        }
    }
    return eAllConnected;
}

void QWoSshTermWidgetImpl::stopSession()
{
    QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->stop();
}

void QWoSshTermWidgetImpl::reconnectSession(bool all)
{
    if(!all) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->reconnect(true);
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(!term->isConnected()) {
            term->reconnect(true);
        }
    }
}

QWoShowerWidget::EHistoryFileState QWoSshTermWidgetImpl::historyFileState()
{
    QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->hasHistoryFile()) {
        return eNoFile;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(*it);
        if(term == nullptr || !term->hasHistoryFile()) {
            return eOtherNoFile;
        }
    }
    return eAllHasFiles;
}

void QWoSshTermWidgetImpl::outputHistoryToFile()
{
    QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->outputHistoryToFile();
}

void QWoSshTermWidgetImpl::stopOutputHistoryToFile(bool all)
{
    if(!all) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->stopOutputHistoryFile();
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoSshTermWidget *term = qobject_cast<QWoSshTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(term->hasHistoryFile()) {
            term->stopOutputHistoryFile();
        }
    }
}

void QWoSshTermWidgetImpl::onSftpAssistOpen()
{
    if(m_sftp == nullptr) {
        m_sftp = new QWoSftpWidget(m_target, m_gid, true, this);
        addAssistant(m_sftp, true);
        m_sftp->setMinimumWidth(30);
        m_sftp->setVisible(true);
    }else{
        m_sftp->deleteLater();
    }
}

void QWoSshTermWidgetImpl::onNewSftpSession()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSftp(m_target, m_gid);
}

void QWoSshTermWidgetImpl::onNewSshSession()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSsh(m_target, m_gid);
}

void QWoSshTermWidgetImpl::onActivePathArrived(const QString &path)
{
    if(m_sftp != nullptr) {
        m_sftp->tryToSyncPath(path);
    }
}
