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

QKxButtonAssist::QKxButtonAssist(const QString& icon, bool frame, QWidget *parent)
    : QObject(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(parent);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(2, 2, 2, 2);
    parent->setLayout(layout);
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_layout = layout;
    append(icon, frame);
}

QKxButtonAssist::QKxButtonAssist(const QString &icon, QWidget *parent)
    : QKxButtonAssist(icon, true, parent)
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
    m_layout->insertWidget(1, btn);
    m_btns.append(btn);
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
    m_layout->insertWidget(1, seperator);
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

void QKxButtonAssist::setIconMargins(int m)
{
    m_layout->setContentsMargins(m,m,m,m);
}

void QKxButtonAssist::onClicked()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    QVariant v = btn->property("index");
    emit clicked(v.toInt());
    emit pressed(btn);
}
