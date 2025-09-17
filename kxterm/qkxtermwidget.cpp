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

#include "qkxtermwidget.h"
#include "qkxtermitem.h"
#include "qkxsearch.h"

#include <QGuiApplication>
#include <QResizeEvent>
#include <QScrollBar>
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QClipboard>

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
    QObject::connect(m_term, SIGNAL(activePathArrived(QString)), this, SIGNAL(activePathArrived(QString)));
    QObject::connect(m_term, SIGNAL(sendData(QByteArray)), this, SIGNAL(sendData(QByteArray)));
    QObject::connect(m_term, SIGNAL(readOnlyChanged()), this, SIGNAL(readOnlyChanged()));
    QObject::connect((QKxTermItem*)m_term, &QKxTermItem::showFindTool, this, [=](){
        setFindBarVisible(true);
    });
    QObject::connect(m_vscroll, SIGNAL(valueChanged(int)), this, SLOT(onScrollValueChanged(int)));
    m_term->installEventFilter(this);
}

QKxTermWidget::~QKxTermWidget()
{
    if(m_vscroll) {
        delete m_vscroll;
    }
    if(m_find) {
        delete m_find;
    }
    if(m_term) {
        delete m_term;
    }
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

void QKxTermWidget::findNext()
{
    QMetaObject::invokeMethod(m_find, "onFindNext");
}

void QKxTermWidget::findPreview()
{
    QMetaObject::invokeMethod(m_find, "onFindPrev");
}

void QKxTermWidget::findAll()
{
    QMetaObject::invokeMethod(m_find, "onFindAll");
}

QString QKxTermWidget::termName() const
{
    return m_term->termName();
}

void QKxTermWidget::setTermName(const QString &name)
{
    m_term->setTermName(name);
}

void QKxTermWidget::sendInput(const QByteArray &cmd)
{
    emit m_term->sendData(cmd);
}

QString QKxTermWidget::textCodec() const
{
    return m_term->textCodec();
}

void QKxTermWidget::setTextCodec(const QString &codec)
{
    m_term->setTextCodec(codec);
}

bool QKxTermWidget::readOnly() const
{
    return m_term->readOnly();
}

void QKxTermWidget::setReadOnly(bool on)
{
    m_term->setReadOnly(on);
}

bool QKxTermWidget::canCopy() const
{
    return m_term->canCopy();
}

bool QKxTermWidget::canPaste() const
{
    return m_term->canPaste();
}

void QKxTermWidget::tryToCopy()
{
    m_term->tryToCopy();
}

void QKxTermWidget::tryToPaste()
{
    m_term->tryToPaste();
}

void QKxTermWidget::pastePlainText(const QString &txt)
{
    m_term->pastePlainText(txt);
}

bool QKxTermWidget::copyWhenOverSelectionText(const QPoint& pt, bool paste)
{
    if(m_term->isOverSelection(pt)) {
        QString txtSel = m_term->selectedText();
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(txtSel);
        if(paste) {
            m_term->directSendData(txtSel.toUtf8());
        }
        return true;
    }
    return false;
}

QString QKxTermWidget::selectedText() const
{
    return m_term->selectedText();
}

void QKxTermWidget::selectAllText()
{
    m_term->selectAll();
}

void QKxTermWidget::clearScreen()
{
    m_term->cleanScreen();
    m_term->scrollToEnd();
    QMetaObject::invokeMethod(m_term, "update", Qt::QueuedConnection);
}

void QKxTermWidget::clearHistory()
{
    m_term->cleanHistory();
    m_term->scrollToEnd();
    QMetaObject::invokeMethod(m_term, "update", Qt::QueuedConnection);
}

void QKxTermWidget::clearAll()
{
    m_term->cleanAll();
    m_term->scrollToEnd();
    QMetaObject::invokeMethod(m_term, "update", Qt::QueuedConnection);
}

void QKxTermWidget::resetTermSize()
{
    m_term->resetTermSize();
}

void QKxTermWidget::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    QSize sz = ev->size();
    QSize szh = m_vscroll->sizeHint();
}

bool QKxTermWidget::eventFilter(QObject *watched, QEvent *event)
{
    return QWidget::eventFilter(watched, event);
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
