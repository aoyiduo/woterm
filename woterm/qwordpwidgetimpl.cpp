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

#include "qwordpwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwordpwidget.h"
#include "qwosshconf.h"

#include <QVBoxLayout>
#include <QMenu>

QWoRdpWidgetImpl::QWoRdpWidgetImpl(const QString& target, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, eRdp, parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    m_rdp = new QWoRdpWidget(target, this);
    QObject::connect(m_rdp, SIGNAL(destroyed()), this, SLOT(onRootDestroy()));
    layout->addWidget(m_rdp);
    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
}

QWoRdpWidgetImpl::~QWoRdpWidgetImpl()
{

}

bool QWoRdpWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoRdpWidgetImpl::handleTabContextMenu(QMenu *menu)
{
    QAction *act = menu->addAction(tr("Smart resize"), this, SLOT(onSmartResize()));
    act->setCheckable(true);
    act->setChecked(m_rdp->smartResize());
}

QMap<QString, QString> QWoRdpWidgetImpl::collectUnsafeCloseMessage()
{
    return QMap<QString, QString>();
}

QWoShowerWidget::ESessionState QWoRdpWidgetImpl::sessionState()
{
    if(m_rdp == nullptr) {
        return eDisconnected;
    }
    return m_rdp->isConnected() ? eAllConnected : eDisconnected;
}

void QWoRdpWidgetImpl::stopSession()
{
    m_rdp->stop();
}

void QWoRdpWidgetImpl::reconnectSession(bool all)
{
    m_rdp->reconnect();
}

void QWoRdpWidgetImpl::onRootDestroy()
{
    deleteLater();
}

void QWoRdpWidgetImpl::onSmartResize()
{
    QAction *action = qobject_cast<QAction*>(sender());

    m_rdp->setSmartResize(action->isChecked());
}
