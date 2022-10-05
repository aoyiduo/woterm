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
    return w;
}
