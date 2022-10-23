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

QKxButtonAssist::QKxButtonAssist(const QString& icon, QWidget *parent)
    : QObject(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(parent);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(2, 2, 2, 2);
    parent->setLayout(layout);
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_layout = layout;
    append(icon);
}

void QKxButtonAssist::append(const QString &icon)
{
    QWidget *p = qobject_cast<QWidget*>(parent());
    QToolButton *btn = new QToolButton(p);
    btn->setIcon(QIcon(icon));
    btn->setProperty("index", m_layout->count() - 1);
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onClicked()));
    m_layout->addWidget(btn);
    m_btns.append(btn);
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
}
