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

#include "qwoshowerwidget.h"


QWoShowerWidget::QWoShowerWidget(const QString &target, QWidget *parent)
    : QWoWidget (parent)
    , m_id(0)
    , m_target(target)
{
    setAttribute(Qt::WA_DeleteOnClose);
}
