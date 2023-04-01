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
