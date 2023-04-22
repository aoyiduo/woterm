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

#include "qmovncwidgetimpl.h"
#include "qmovncwidget.h"
#include "qmomenu.h"
#include "qkxtouchpoint.h"
#include "qkxmessagebox.h"
#include "qmokeyboard.h"

#include <QResizeEvent>
#include <QScrollArea>

#define MID_MANUAL      (QMoMenu::MID_User+1)
#define MID_KEYBOARD      (QMoMenu::MID_User+2)

QMoVncWidgetImpl::QMoVncWidgetImpl(const QString &target, QWidget *parent)
    : QMoShowWidget(target, parent)
{
    m_area = new QScrollArea(this);
    if(m_area) {
        m_area->setFrameShape(QFrame::NoFrame);
        m_area->setContentsMargins(0, 0, 0, 0);
        m_player = new QMoVncWidget(target, m_area);
        m_area->setWidget(m_player);
        m_player->resize(1024, 768);
    }else{
        m_player = new QMoVncWidget(target, this);
    }
    showTouchPoint(true, true);
    QObject::connect(m_player, SIGNAL(forceToClose()), this, SLOT(onForceToClose()));
}

QMoVncWidgetImpl::~QMoVncWidgetImpl()
{
    if(m_area) {
        delete m_area;
    }
}

void QMoVncWidgetImpl::resizeEvent(QResizeEvent *ev)
{
    QMoShowWidget::resizeEvent(ev);
    if(m_area) {
        m_area->resize(ev->size());
    }
    QMetaObject::invokeMethod(this, "resetTouchPointPosition", Qt::QueuedConnection);
}

void QMoVncWidgetImpl::onForceToClose()
{
    close();
}

void QMoVncWidgetImpl::onTouchPointClicked()
{
    if(m_menu == nullptr) {
        m_menu = new QMoMenu(this);
        QObject::connect(m_menu, SIGNAL(clicked(int)), this, SLOT(onMenuItemClickedArrived(int)));
    }
    if(m_keyboard && m_keyboard->isVisible()) {
        m_menu->addItem(MID_KEYBOARD, tr("Close keyboard"), ":/woterm/resource/skin/keyboard.png");
    }else{
        m_menu->addItem(MID_KEYBOARD, tr("Open keyboard"), ":/woterm/resource/skin/keyboard.png");
    }
    m_menu->addItem(MID_MANUAL, tr("Manual"), ":/woterm/resource/skin/help.png");
    m_menu->setFixedSize(size());
    m_menu->show();
}

void QMoVncWidgetImpl::onMenuItemClickedArrived(int mid)
{
    if(mid == QMoMenu::MID_Back) {
        close();
    }else if(mid == MID_KEYBOARD) {
        if(m_keyboard == nullptr) {
            m_keyboard = new QMoKeyboard(this);
            m_keyboard->setVNCPatch(true);
            m_keyboard->setDragEnabled(true);
            QObject::connect(m_keyboard, SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onKeyboardKeyEvent(QKeyEvent*)));
        }
        m_keyboard->setMaximumSize(size());
        m_keyboard->adjustSize();
        m_keyboard->setVisible(!m_keyboard->isVisible());
    }else if(mid == MID_MANUAL) {
        QStringList helps;
        helps.append(tr("Drag the screen with two fingers"));
        QKxMessageBox::information(this, tr("Manual"), helps.join("\r\n"));
    }
}

void QMoVncWidgetImpl::onKeyboardKeyEvent(QKeyEvent *ev)
{
    m_player->handleKeyEvent(ev);
}

void QMoVncWidgetImpl::showTouchPoint(bool show, bool async)
{
    if(async == true) {
        QMetaObject::invokeMethod(this, "showTouchPoint", Qt::QueuedConnection, Q_ARG(bool, show), Q_ARG(bool, false));
        return;
    }
    if(show) {
        if(m_touchPoint == nullptr) {
            m_touchPoint = new QKxTouchPoint(this);            
            QObject::connect(m_touchPoint, SIGNAL(clicked()), this, SLOT(onTouchPointClicked()));
        }
        QMetaObject::invokeMethod(this, "resetTouchPointPosition", Qt::QueuedConnection);
        m_touchPoint->raise();
        m_touchPoint->show();
    }else if(m_touchPoint) {
        m_touchPoint->hide();
    }
}

void QMoVncWidgetImpl::resetTouchPointPosition()
{
    if(m_touchPoint) {
        int tw = m_touchPoint->raduis() * 2;
        QSize sz = size();
        QRect rt(sz.width() - tw, (sz.height() - tw) / 2, tw, tw);
        m_touchPoint->setGeometry(rt);
    }
}

