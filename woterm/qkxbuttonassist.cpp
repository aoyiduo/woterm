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

#include "qkxbuttonassist.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QVariant>
#include <QFrame>
#include <QSpacerItem>
#include <QResizeEvent>

QKxButtonAssist::QKxButtonAssist(const QString& icon, bool frame, QWidget *parent)
    : QObject(parent)
    , m_margins(2)
    , m_spacing(1)
    , m_frameWidth(2)
{
    parent->installEventFilter(this);
    append(icon, frame);
}

QKxButtonAssist::QKxButtonAssist(const QString &icon, QWidget *parent)
    : QKxButtonAssist(icon, true, parent)
{

}

void QKxButtonAssist::setMargins(int n)
{
    m_margins = n;
}

void QKxButtonAssist::setSpacing(int n)
{

}

QToolButton *QKxButtonAssist::append(const QString &icon, bool frame)
{
    QWidget *p = qobject_cast<QWidget*>(parent());
    QToolButton *btn = new QToolButton(p);
    btn->setObjectName("buttonAssist");
    btn->setIcon(QIcon(icon));
    btn->setProperty("index", m_btns.length());
    if(!frame){
        QString style = "QToolButton{border:0;border-radius:3px;}\r\n";
        style += "QToolButton:hover{background-color:rgba(255,255,255,128)}\r\n";
        style += "QToolButton:pressed{background-color:rgba(200,200,200,128)}";
        btn->setStyleSheet(style);
    }
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onClicked()));
    m_btns.insert(0, btn);
    m_childs.insert(0, btn);
    QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
    return btn;
}

QToolButton *QKxButtonAssist::button(int idx)
{
    if(idx < 0 || idx >= m_btns.length()) {
        return nullptr;
    }
    return m_btns[idx];
}

void QKxButtonAssist::appendSeperator()
{
    QWidget *p = qobject_cast<QWidget*>(parent());
    QFrame *seperator = new QFrame(p);
    seperator->setFrameShape(QFrame::VLine);
    seperator->setFrameShadow(QFrame::Sunken);
    seperator->setObjectName("buttonAssistVLine");    
    m_childs.insert(0, seperator);
    QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
}

void QKxButtonAssist::setEnabled(int idx, bool on)
{
    if(idx < 0 || idx >= m_btns.length()) {
        return;
    }
    QToolButton* btn = m_btns.at(idx);
    btn->setEnabled(on);
}

bool QKxButtonAssist::isEnabled(int idx)
{
    if(idx < 0 || idx >= m_btns.length()) {
        return false;
    }
    QToolButton* btn = m_btns.at(idx);
    return btn->isEnabled();
}

void QKxButtonAssist::onClicked()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    QVariant v = btn->property("index");
    emit clicked(v.toInt());
    emit pressed(btn);
}

bool QKxButtonAssist::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize) {
        QResizeEvent *ev = (QResizeEvent*)event;
        QSize sz = ev->size();
        QMetaObject::invokeMethod(this, "resetPosition", Qt::QueuedConnection);
    }
    return false;
}

void QKxButtonAssist::resetPosition()
{
    QWidget *widget = qobject_cast<QWidget*>(parent());
    QSize sz = widget->size();
    int bh = sz.height() - m_margins - m_margins;

    int bx = sz.width() - bh * m_btns.length() - (m_childs.length() - m_btns.length()) * m_frameWidth - (m_childs.length() - 1) * m_spacing - m_margins;
    for(int i = 0; i < m_childs.size(); i++) {
        QWidget *w = m_childs.at(i);
        QToolButton *btn = qobject_cast<QToolButton*>(w);
        if(btn) {
            QRect brt(bx, m_margins, bh, bh);
            btn->setFixedSize(bh, bh);
            btn->setGeometry(brt);
            bx += bh;
        }else{
            QRect brt(bx, m_margins, m_frameWidth, bh);
            w->setGeometry(brt);
            bx += m_frameWidth;
        }
        bx += m_spacing;
    }
}
