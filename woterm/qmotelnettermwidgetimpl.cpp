/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qmotelnettermwidgetimpl.h"
#include "qmotelnettermwidget.h"

QMoTelnetTermWidgetImpl::QMoTelnetTermWidgetImpl(const QString &target, QWidget *parent)
    : QMoTermWidgetImpl(target, parent)
{

}

QMoTermWidget *QMoTelnetTermWidgetImpl::createTermWidget(const QString &target, QWidget *parent)
{
    return new QMoTelnetTermWidget(target, parent);
}
