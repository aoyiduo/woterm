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

#include "qmotermwidgetimpl.h"
#include "qmotermwidget.h"
#include "qmotermkeyboard.h"
#include "qkxtermitem.h"
#include "qmomenu.h"

#include <QVBoxLayout>
#include <QKeyEvent>


QMoTermWidgetImpl::QMoTermWidgetImpl(const QString& target, QWidget *parent)
    : QMoShowWidget(target, parent)
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    setAttribute(Qt::WA_DeleteOnClose);
}

QMoTermWidgetImpl::~QMoTermWidgetImpl()
{

}

void QMoTermWidgetImpl::showKeyboard(bool on)
{
    m_keyboard->setVisible(on);
}

void QMoTermWidgetImpl::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QMoTermWidget *term = createTermWidget(m_target, this);
    layout->addWidget(term);

    QMoKeyboard *keyboard = new QMoKeyboard(this);
    layout->addWidget(keyboard);
    QObject::connect(keyboard, SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onSimulateKeyEvent(QKeyEvent*)));

    QKxTermItem *item = term->termItem();
    QObject::connect(item, SIGNAL(touchPointClicked()), this, SLOT(onShowMenu()));

    m_term = term;
    m_keyboard = keyboard;
}

void QMoTermWidgetImpl::showMenu()
{

}

void QMoTermWidgetImpl::onShowMenu()
{
    if(m_menu == nullptr) {
        m_menu = new QMoMenu(this);
        m_menu->setFixedSize(size());
        QObject::connect(m_menu, SIGNAL(clicked(int)), this, SLOT(onClickedArrived(int)));
    }
    m_menu->show();
}

void QMoTermWidgetImpl::onClickedArrived(int mid)
{
    if(mid == QMoMenu::MID_Back) {
        close();
    }
}

void QMoTermWidgetImpl::onSimulateKeyEvent(QKeyEvent *ev)
{
    QKxTermItem *item = m_term->termItem();
    if(ev->type() == QEvent::KeyPress) {
        item->simulateKeyPress(ev);
    }else{
        item->simulateKeyRelease(ev);
    }
}
