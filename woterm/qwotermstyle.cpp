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

#include "qwotermstyle.h"

#include <QtWidgets>
#include <QStyleFactory>

QWoTermStyle::QWoTermStyle(const QString& name)
    : QProxyStyle(QStyleFactory::create(name))
{

}

QIcon QWoTermStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    switch (standardIcon) {
    case SP_DialogCloseButton:
        return QIcon(":/woterm/resource/skin/tabclose.png");
    default:
        return QProxyStyle::standardIcon(standardIcon, option, widget);
    }
}

void QWoTermStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_IndicatorTabClose:
        {
            const QStyleOptionTab *opt = qstyleoption_cast<const QStyleOptionTab*>(option);
            QIcon ico = proxy()->standardIcon(SP_DialogCloseButton, option, widget);
            if(ico.isNull()) {
                qDebug() << "ico is ok";
            }
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
        break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void QWoTermStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_TabBarTab:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}
