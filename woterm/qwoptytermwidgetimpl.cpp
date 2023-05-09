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

#include "qwoptytermwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwoptytermwidget.h"
#include "qwosetting.h"
#include "qwosessionttyproperty.h"

QWoPtyTermWidgetImpl::QWoPtyTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoTermWidgetImpl(target, gid, tab, parent)
{
}

QWoPtyTermWidgetImpl::~QWoPtyTermWidgetImpl()
{

}

QWoTermWidget *QWoPtyTermWidgetImpl::createTermWidget(const QString &target, int gid, QWidget *parent)
{
    QWoPtyTermWidget *w = new QWoPtyTermWidget(target, gid, parent);
    QString name = tr("%1:%2").arg(increaseId()).arg(target);
    w->setTermName(name);
    w->setObjectName(name);
    return w;
}

bool QWoPtyTermWidgetImpl::handleCustomProperties()
{
    QVariantMap prop = QWoSetting::localShell();
    QWoSessionTTYProperty dlg(QWoSessionTTYProperty::ETTY_LocalShell, this);
    dlg.setCustom(prop);
    dlg.exec();
    QVariantMap result = dlg.result();
    if(!result.isEmpty()) {
        QWoSetting::setLocalShell(result);
        for(int i = 0; i < m_terms.length(); i++) {
            QWidget *w = m_terms.at(i);
            QMetaObject::invokeMethod(w, "initCustom", Qt::QueuedConnection);
        }
    }
    return true;
}
