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

#include "qmoredpoint.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

QMoRedPoint::QMoRedPoint(const QSize &size, QWidget *parent)
    : QWidget(parent)
    , m_size(size)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(size);
}

void QMoRedPoint::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(Qt::red));
    p.drawEllipse(rect());
    p.restore();
}
