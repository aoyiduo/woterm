/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qmoptytermwidgetimpl.h"
#include "qmoptytermwidget.h"

QMoPtyTermWidgetImpl::QMoPtyTermWidgetImpl(QWidget *parent)
    : QMoTermWidgetImpl("Terminal", parent)
{

}

QMoTermWidget *QMoPtyTermWidgetImpl::createTermWidget(const QString &target, QWidget *parent)
{
    return new QMoPtyTermWidget(target, parent);
}
