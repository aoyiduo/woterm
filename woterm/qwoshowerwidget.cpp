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

#include "qwoshowerwidget.h"
#include "qwosessionproperty.h"
#include "qwosshconf.h"
#include "qkxmessagebox.h"
#include "qwomainwindow.h"
#include "qwofloatwindow.h"
#include "qwoshower.h"

QWoShowerWidget::QWoShowerWidget(const QString &target, EShowerType type, QWidget *parent)
    : QWoWidget (parent)
    , m_id(0)
    , m_target(target)
    , m_typeShower(type)
{
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
}

QWidget *QWoShowerWidget::lastFocusWidget()
{
    return m_lastFocusWidget;
}

void QWoShowerWidget::setLastFocusWidget(QWidget *w)
{
    m_lastFocusWidget = w;
}

bool QWoShowerWidget::handleCustomProperties()
{
    if(!QWoSshConf::instance()->exists(m_target)){
        QKxMessageBox::warning(this, tr("Error"), tr("can't find the session, maybe it had been delete ago"));
        return false;
    }
    QWoSessionProperty dlg(this);
    QObject::connect(&dlg, SIGNAL(readyToConnect(QString,int)), QWoMainWindow::instance(), SLOT(onSessionReadyToConnect(QString,int)));
    dlg.setSession(m_target);
    int ret = dlg.exec();
    return ret == QWoSessionProperty::Save;
}

bool QWoShowerWidget::isRemoteSession() {
    return true;
}

void QWoShowerWidget::onShowFullScreen()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    if(wfloat != nullptr) {
        wfloat->showFullScreen();
        QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
        return;
    }
    QMetaObject::invokeMethod(QWoMainWindow::shower(), "floatSession", Q_ARG(QWoShowerWidget*, this), Q_ARG(bool, true));
}
