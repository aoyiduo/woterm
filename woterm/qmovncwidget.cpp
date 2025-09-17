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

#include "qmovncwidget.h"

#include <QKeyEvent>

QMoVncWidget::QMoVncWidget(const QString &target, QWidget *parent)
    : QWoVncWidget(target, parent)
{

}

void QMoVncWidget::handleKeyEvent(QKeyEvent *ev)
{
    QEvent::Type t = ev->type();
    if(t == QEvent::KeyPress) {
        keyPressEvent(ev);
    }else if(t == QEvent::KeyRelease){
        keyReleaseEvent(ev);
    }
}
