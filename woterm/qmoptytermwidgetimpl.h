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

#ifndef QMOPTYTERMWIDGETIMPL_H
#define QMOPTYTERMWIDGETIMPL_H

#include "qmotermwidgetimpl.h"

class QMoPtyTermWidgetImpl : public QMoTermWidgetImpl
{
    Q_OBJECT
public:
    explicit QMoPtyTermWidgetImpl(QWidget *parent = nullptr);

protected:
    virtual QMoTermWidget *createTermWidget(const QString& target, QWidget *parent);
};

#endif // QMOPTYTERMWIDGETIMPL_H
