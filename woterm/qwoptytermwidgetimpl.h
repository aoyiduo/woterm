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

#pragma once

#include "qwotermwidgetimpl.h"

#include <QPointer>

class QTabBar;

class QWoPtyTermWidgetImpl : public QWoTermWidgetImpl
{
    Q_OBJECT
public:
    explicit QWoPtyTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent=nullptr);
    ~QWoPtyTermWidgetImpl();
protected:
    virtual QWoTermWidget *createTermWidget(const QString& target, int gid, QWidget *parent);
};
