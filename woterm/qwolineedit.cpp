/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwolineedit.h"

#include <QToolButton>
#include <QResizeEvent>

QWoLineEdit *QWoLineEdit::decorator(QLineEdit *parent)
{
    QWoLineEdit *my = new QWoLineEdit(parent);
    return my;
}

QWoLineEdit::QWoLineEdit(QLineEdit *parent)
    : QObject(parent)
    , m_edit(parent)
{
    parent->installEventFilter(this);
    m_btn = new QToolButton(parent);
    m_btn->setIcon(QIcon("../private/skins/black/tabclose.png"));
    QMargins m = m_edit->textMargins();
    m.setRight(m.right() + 16);
    m_edit->setTextMargins(m);
    QObject::connect(m_btn, SIGNAL(clicked()), m_edit, SLOT(clear()));
}

bool QWoLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    if(m_edit == watched){
        if(event->type() == QEvent::Resize) {
            QResizeEvent *ev = (QResizeEvent*)event;
            QSize sz = ev->size();
            m_btn->setGeometry(sz.width() - sz.height(), 1, sz.height() - 1, sz.height() - 2);
        }
    }
    return false;
}
