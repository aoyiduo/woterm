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

#include "qwoplaybookwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwoplaybookwidget.h"
#include "qwosshconf.h"

#include <QVBoxLayout>
#include <QMenu>

QWoPlaybookWidgetImpl::QWoPlaybookWidgetImpl(const QString &path, const QString& name, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(name, ePlaybook, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_playbook = new QWoPlaybookWidget(path, name, this);
    QObject::connect(m_playbook, SIGNAL(destroyed()), this, SLOT(onRootDestroy()));
    layout->addWidget(m_playbook);
    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
}

QWoPlaybookWidgetImpl::~QWoPlaybookWidgetImpl()
{

}

bool QWoPlaybookWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoPlaybookWidgetImpl::handleTabContextMenu(QMenu *menu)
{
    if(m_playbook->isConsoleVisible()) {
        //menu->addAction(tr("Set console foreground"), this, SLOT(onOpenConsoleWindow()));
        menu->addAction(tr("Close console window"), this, SLOT(onCloseConsoleWindow()));
    }else{
        menu->addAction(tr("Open console window"), this, SLOT(onOpenConsoleWindow()));
    }
}

bool QWoPlaybookWidgetImpl::handleCustomProperties()
{
    m_playbook->setDebugMode(true);
    return true;
}

QMap<QString, QString> QWoPlaybookWidgetImpl::collectUnsafeCloseMessage()
{
    return QMap<QString, QString>();
}

bool QWoPlaybookWidgetImpl::isRemoteSession()
{
    return false;
}

void QWoPlaybookWidgetImpl::onRootDestroy()
{
    deleteLater();
}

void QWoPlaybookWidgetImpl::onEnterScriptDebugTriggered()
{
    m_playbook->setDebugMode(true);
}

void QWoPlaybookWidgetImpl::onLeaveScriptDebugTriggered()
{
    m_playbook->setDebugMode(false);
}

void QWoPlaybookWidgetImpl::onOpenConsoleWindow()
{
    m_playbook->openConsole();
}

void QWoPlaybookWidgetImpl::onCloseConsoleWindow()
{
    m_playbook->closeConsole();
}
