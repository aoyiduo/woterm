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

#include "qkxplaintextedit.h"

#include <QMouseEvent>
#include <QDebug>

QKxPlainTextEdit::QKxPlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover,true);
}

QKxPlainTextEdit::~QKxPlainTextEdit()
{

}

void QKxPlainTextEdit::mousePressEvent(QMouseEvent *e)
{
    m_clickedAnchor = (e->button() & Qt::LeftButton) ? anchorAt(e->pos()) : QString();
    QPlainTextEdit::mousePressEvent(e);
}

void QKxPlainTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    QString link = anchorAt(e->pos());
    if(link.isEmpty()) {
        unsetCursor();
    }else{
        setCursor(Qt::ArrowCursor);
    }
    QPlainTextEdit::mouseMoveEvent(e);
}

void QKxPlainTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton && !m_clickedAnchor.isEmpty() && anchorAt(e->pos()) == m_clickedAnchor) {
        emit linkActivated(m_clickedAnchor);
    }

    QPlainTextEdit::mouseReleaseEvent(e);
}

bool QKxPlainTextEdit::event(QEvent *e)
{
    if(QEvent::HoverMove == e->type()){
        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(e);
        QString link = anchorAt(hoverEvent->pos());
        if(link.isEmpty()) {
            unsetCursor();
        }else{
            setCursor(Qt::ArrowCursor);
        }
        qDebug() << hoverEvent->pos() << "anchor: " << link;
    }
    return QWidget::event(e);
}
