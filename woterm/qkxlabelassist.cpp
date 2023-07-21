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

#include "qkxlabelassist.h"

#include <QResizeEvent>

QKxLabelAssist::QKxLabelAssist(QWidget *parent)
    : QObject(parent)
{
    m_label = new QLabel(parent);
    m_label->setAlignment(Qt::AlignCenter);
    QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
    parent->installEventFilter(this);
}

void QKxLabelAssist::setTextAlignment(Qt::Alignment align)
{
    m_label->setAlignment(align);
}

void QKxLabelAssist::setText(const QString &txt)
{
    m_label->setText(txt);
}

void QKxLabelAssist::setVisible(bool on)
{
    m_label->setVisible(on);
}

bool QKxLabelAssist::isVisible()
{
    return m_label->isVisible();
}

bool QKxLabelAssist::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize) {
        QResizeEvent *ev = (QResizeEvent*)event;
        QSize sz = ev->size();
        QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
    }
    return false;
}

void QKxLabelAssist::resetPosition()
{
    QWidget *widget = qobject_cast<QWidget*>(parent());
    QSize sz = widget->size();
    m_label->setGeometry(QRect(0, 0, sz.width(), sz.height()));
}
