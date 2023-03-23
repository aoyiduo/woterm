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
