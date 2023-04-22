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

#include "qmorlogintermwidgetimpl.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qmorlogintermwidget.h"


QMoRLoginTermWidgetImpl::QMoRLoginTermWidgetImpl(const QString& target,  QWidget *parent)
    : QMoTermWidgetImpl(target,  parent)
{
}

QMoRLoginTermWidgetImpl::~QMoRLoginTermWidgetImpl()
{

}

QMoTermWidget *QMoRLoginTermWidgetImpl::createTermWidget(const QString &target, QWidget *parent)
{
    QMoRLoginTermWidget *w = new QMoRLoginTermWidget(target, parent);
    w->setTermName(target);
    w->setObjectName(target);
    return w;
}
