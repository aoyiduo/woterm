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

#include "qwotelnettermwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwotelnettermwidget.h"


QWoTelnetTermWidgetImpl::QWoTelnetTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoTermWidgetImpl(target, gid, tab, parent)
{
}

QWoTelnetTermWidgetImpl::~QWoTelnetTermWidgetImpl()
{

}

QWoTermWidget *QWoTelnetTermWidgetImpl::createTermWidget(const QString &target, int gid, QWidget *parent)
{
    QWoTelnetTermWidget *w = new QWoTelnetTermWidget(target, gid, parent);
    QString name = tr("%1:%2").arg(increaseId()).arg(target);
    w->setTermName(name);
    w->setObjectName(name);
    w->setImplementWidget(this);
    return w;
}

QWoShowerWidget::ESessionState QWoTelnetTermWidgetImpl::sessionState()
{
    QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->isConnected()) {
        return eDisconnected;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(*it);
        if(term == nullptr || !term->isConnected()) {
            return eOtherDisconnected;
        }
    }
    return eAllConnected;
}

void QWoTelnetTermWidgetImpl::stopSession()
{
    QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->stop();
}

void QWoTelnetTermWidgetImpl::reconnectSession(bool all)
{
    if(!all) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->reconnect();
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(!term->isConnected()) {
            term->reconnect();
        }
    }
}

QWoShowerWidget::EHistoryFileState QWoTelnetTermWidgetImpl::historyFileState()
{
    QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
    if(term == nullptr || !term->hasHistoryFile()) {
        return eNoFile;
    }

    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(*it);
        if(term == nullptr || !term->hasHistoryFile()) {
            return eOtherNoFile;
        }
    }
    return eAllHasFiles;
}

void QWoTelnetTermWidgetImpl::outputHistoryToFile()
{
    QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
    if(term == nullptr) {
        return;
    }
    term->outputHistoryToFile();
}

void QWoTelnetTermWidgetImpl::stopOutputHistoryToFile(bool all)
{
    if(!all) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(lastFocusWidget());
        if(term == nullptr) {
            return;
        }
        term->stopOutputHistoryFile();
        return;
    }
    for(auto it = m_terms.begin(); it != m_terms.end(); it++) {
        QWoTelnetTermWidget *term = qobject_cast<QWoTelnetTermWidget*>(*it);
        if(term == nullptr) {
            continue;
        }
        if(term->hasHistoryFile()) {
            term->stopOutputHistoryFile();
        }
    }
}
