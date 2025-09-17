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

#ifndef QMOVNCWIDGET_H
#define QMOVNCWIDGET_H

#include "qwovncwidget.h"
#include <QPointer>

class QMoVncWidget : public QWoVncWidget
{
    Q_OBJECT
public:
    explicit QMoVncWidget(const QString& target, QWidget *parent = nullptr);
    void handleKeyEvent(QKeyEvent* ev);
};

#endif // QMOVNCWIDGET_H
