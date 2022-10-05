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

#ifndef QWOTERMSTYLE_H
#define QWOTERMSTYLE_H

#include <QProxyStyle>
#include <QtWidgets>

class QWoTermStyle : public QProxyStyle
{
    Q_OBJECT
public:
    QWoTermStyle(const QString& name);

protected:
    QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
private:

};

#endif // QWOTERMSTYLE_H
