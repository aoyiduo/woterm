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

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxsearch.h"

#include <QGuiApplication>
#include <QResizeEvent>
#include <QScrollBar>
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>

const QString scrollbar_valid =  \
"QScrollBar::vertical \
{\
    margin:0px 0px 0px 0px;\
    padding-top:0px;\
    padding-bottom:0px;\
    padding-left:1px;\
    padding-right:1px;\
    background: rgba(255,255,255,128);\
    border:1px solid gray;\
    width:12px;\
}\
QScrollBar::handle:vertical \
{\
    background: lightGray;\
    border: 1px solid gray;\
    border-radius:3px;\
    width:8px;\
    min-width:8px;\
    min-height: 30px;\
}\
QScrollBar::handle:vertical:hover\
{\
    background: rgb(18,150,219);\
    border: 1px solid gray;\
    border-radius:3px;\
    width:8px;\
    min-width:8px;\
    min-height: 30px;\
}\
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical \
{\
    width:0px;\
    height:0px;\
    background:none;\
}\
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical\
{\
    background: none;\
}";

const QString scrollbar_invalid =  \
"QScrollBar::vertical \
{\
    margin:0px 0px 0px 0px;\
    padding-top:0px;\
    padding-bottom:0px;\
    padding-left:1px;\
    padding-right:1px;\
    background: rgba(255,255,255,128);\
    border:1px solid gray;\
    width:12px;\
}\
QScrollBar::handle:vertical \
{\
    background: none;\
}\
QScrollBar::handle:vertical:hover\
{\
    background: none;\
}\
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical \
{\
    width:0px;\
    height:0px;\
    background:none;\
}\
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical\
{\
    background: none;\
}";

QKxTermWidget::QKxTermWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    setLayout(hbox);
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->setSpacing(0);
    hbox->addLayout(vbox);
    m_term = new QKxTermItem(this);
    vbox->addWidget(m_term);
    m_find = new QKxSearch(m_term, this);
    vbox->addWidget(m_find);
    m_find->setVisible(false);
    m_vscroll = new QScrollBar(m_term);
    m_vscroll->setRange(0, 0);
    hbox->addWidget(m_vscroll);
    m_vscroll->setStyleSheet(scrollbar_invalid);
    QObject::connect(m_term, SIGNAL(scrollValueChanged(int,int)), this, SLOT(onTermScrollValueChanged(int,int)));
    QObject::connect(m_vscroll, SIGNAL(valueChanged(int)), this, SLOT(onScrollValueChanged(int)));
    m_term->installEventFilter(this);
    m_keyFind = QKeySequence(Qt::CTRL + Qt::Key_F);
}

QKxTermItem *QKxTermWidget::termItem()
{
    return m_term;
}

QWidget *QKxTermWidget::findBar()
{
    return m_find;
}

void QKxTermWidget::setFindBarVisible(bool on)
{
    m_find->setVisible(on);
}

QString QKxTermWidget::termName() const
{
    return m_term->termName();
}

void QKxTermWidget::setTermName(const QString &name)
{
    m_term->setTermName(name);
}

void QKxTermWidget::setFindShortCut(QKeySequence key)
{
    m_keyFind = key;
}

void QKxTermWidget::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    QSize sz = ev->size();
    QSize szh = m_vscroll->sizeHint();
    //m_vscroll->setGeometry(sz.width() - szh.width(), 0, szh.width(), sz.height());
    //m_term->setGeometry(0, 0, sz.width() - szh.width(), sz.height());
}

bool QKxTermWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_term) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *ev = (QKeyEvent*)event;
            int key = ev->key();
            Qt::KeyboardModifiers modifier = ev->modifiers();
            if(modifier & Qt::ShiftModifier) {
                key += Qt::SHIFT;
            }
            if(modifier & Qt::ControlModifier) {
                key += Qt::CTRL;
            }
            if(modifier & Qt::MetaModifier) {
                key += Qt::META;
            }
            if(modifier & Qt::AltModifier) {
                key += Qt::ALT;
            }
            QKeySequence seq(key);
            if(seq == m_keyFind) {
                setFindBarVisible(true);
            }
        }
    }
    return false;
}

void QKxTermWidget::onTermScrollValueChanged(int lines, int position)
{
    QVariant v = m_vscroll->property("scrollbar_style");
    if(lines == 0) {
        if(v.toBool()) {
            m_vscroll->setStyleSheet(scrollbar_invalid);
            m_vscroll->setProperty("scrollbar_style", false);
        }
    }else if(!v.toBool()){
        m_vscroll->setStyleSheet(scrollbar_valid);
        m_vscroll->setProperty("scrollbar_style", true);
    }
    if(m_vscroll->maximum() != lines) {
        m_vscroll->setRange(0, lines);
    }
    if(m_vscroll->value() != position){
        m_vscroll->setValue(position);
    }
}

void QKxTermWidget::onScrollValueChanged(int position)
{
    if(m_term){
        m_term->scrollTo(position);
    }
}
