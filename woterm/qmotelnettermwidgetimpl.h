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

#ifndef QMOTELNETTERMWIDGETIMPL_H
#define QMOTELNETTERMWIDGETIMPL_H

#include "qmotermwidgetimpl.h"

class QMoTelnetTermWidgetImpl : public QMoTermWidgetImpl
{
    Q_OBJECT
public:
    explicit QMoTelnetTermWidgetImpl(const QString& target, QWidget *parent = nullptr);

protected:
    virtual QMoTermWidget *createTermWidget(const QString& target, QWidget *parent);
};

#endif // QMOTELNETTERMWIDGETIMPL_H
