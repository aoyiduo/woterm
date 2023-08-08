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

#include "qworlogintermwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qworlogintermwidget.h"


QWoRLoginTermWidgetImpl::QWoRLoginTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoTermWidgetImpl(target, gid, tab, parent)
{
}

QWoRLoginTermWidgetImpl::~QWoRLoginTermWidgetImpl()
{

}

QWoTermWidget *QWoRLoginTermWidgetImpl::createTermWidget(const QString &target, int gid, QWidget *parent)
{
    QWoRLoginTermWidget *w = new QWoRLoginTermWidget(target, gid, parent);
    QString name = tr("%1:%2").arg(increaseId()).arg(target);
    w->setTermName(name);
    w->setObjectName(name);
    w->setImplementWidget(this);
    return w;
}

QWoShowerWidget::ESessionState QWoRLoginTermWidgetImpl::sessionState()
{
    QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->isConnected()) {
        return eDisconnected;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(*it);
        if(term == nullptr || !term->isConnected()) {
            return eOtherDisconnected;
        }
    }
    return eAllConnected;
}

void QWoRLoginTermWidgetImpl::stopSession()
{
    QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->stop();
}

void QWoRLoginTermWidgetImpl::reconnectSession(bool all)
{
    if(!all) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->reconnect(true);
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(!term->isConnected()) {
            term->reconnect(true);
        }
    }
}

QWoShowerWidget::EHistoryFileState QWoRLoginTermWidgetImpl::historyFileState()
{
    QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->hasHistoryFile()) {
        return eNoFile;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(*it);
        if(term == nullptr || !term->hasHistoryFile()) {
            return eOtherNoFile;
        }
    }
    return eAllHasFiles;
}

void QWoRLoginTermWidgetImpl::outputHistoryToFile()
{
    QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->outputHistoryToFile();
}

void QWoRLoginTermWidgetImpl::stopOutputHistoryToFile(bool all)
{
    if(!all) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->stopOutputHistoryFile();
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoRLoginTermWidget *term = qobject_cast<QWoRLoginTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(term->hasHistoryFile()) {
            term->stopOutputHistoryFile();
        }
    }
}
