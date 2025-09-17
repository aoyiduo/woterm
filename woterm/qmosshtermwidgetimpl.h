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

#ifndef QMOSSHTERMWIDGETIMPL_H
#define QMOSSHTERMWIDGETIMPL_H

#include "qmotermwidgetimpl.h"

class QMoSshTermWidget;
class QMoSshTermWidgetImpl : public QMoTermWidgetImpl
{
    Q_OBJECT
public:
    explicit QMoSshTermWidgetImpl(const QString& target, QWidget *parent = nullptr);

protected:
    virtual QMoTermWidget *createTermWidget(const QString& target, QWidget *parent);

};

#endif // QMOSSHTERMWIDGETIMPL_H
