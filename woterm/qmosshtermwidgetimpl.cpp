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

#include "qmosshtermwidgetimpl.h"

#include "qmosshtermwidget.h"

QMoSshTermWidgetImpl::QMoSshTermWidgetImpl(const QString& target, QWidget *parent)
    : QMoTermWidgetImpl(target, parent)
{

}

QMoTermWidget *QMoSshTermWidgetImpl::createTermWidget(const QString &target, QWidget *parent)
{
    return new QMoSshTermWidget(target, parent);
}
