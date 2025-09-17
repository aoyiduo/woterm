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

#include "qwovncwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwovncplaywidget.h"
#include "qwosshconf.h"

#include <QVBoxLayout>
#include <QMenu>

QWoVncWidgetImpl::QWoVncWidgetImpl(const QString& target, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, eVnc, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_vnc = new QWoVncPlayWidget(target, this);
    QObject::connect(m_vnc, SIGNAL(destroyed()), this, SLOT(onRootDestroy()));
    layout->addWidget(m_vnc);
    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
}

QWoVncWidgetImpl::~QWoVncWidgetImpl()
{

}

bool QWoVncWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoVncWidgetImpl::handleTabContextMenu(QMenu *menu)
{
    QAction *act = menu->addAction(tr("Smart resize"), this, SLOT(onSmartResize()));
    act->setCheckable(true);
    act->setChecked(m_vnc->smartResize());
}

QMap<QString, QString> QWoVncWidgetImpl::collectUnsafeCloseMessage()
{
    return QMap<QString, QString>();
}

QWoShowerWidget::ESessionState QWoVncWidgetImpl::sessionState()
{
    if(m_vnc == nullptr) {
        return eDisconnected;
    }
    return m_vnc->isConnected() ? eAllConnected : eDisconnected;
}

void QWoVncWidgetImpl::stopSession()
{
    m_vnc->stop();
}

void QWoVncWidgetImpl::reconnectSession(bool all)
{
    m_vnc->reconnect();
}

void QWoVncWidgetImpl::onRootDestroy()
{
    deleteLater();
}

void QWoVncWidgetImpl::onSmartResize()
{
    m_vnc->setSmartResize(!m_vnc->smartResize());
}
