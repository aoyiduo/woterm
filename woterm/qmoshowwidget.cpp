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

#include "qmoshowwidget.h"

#include <QTimer>
QMoShowWidget::QMoShowWidget(const QString &target, QWidget *parent)
    : QWidget(parent)
    , m_target(target)
{
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);    

    // do not why it could not same on android device
    // so just to try to fix it again later about 1 second.
    QTimer::singleShot(500, this, SLOT(onResizeAgaint()));
}

void QMoShowWidget::onResizeAgaint()
{
    QWidget *pw = parentWidget();
    QSize sz = pw->size();
    QSize szHit = this->size();
    if(sz != szHit) {
        move(0, 0);
        resize(sz);
    }
}
