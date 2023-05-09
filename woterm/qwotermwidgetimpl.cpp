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

#include "qwotermwidgetimpl.h"
#include "qwotermwidget.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwosshconf.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qwocommandlineinput.h"
#include "qwoutils.h"
#include "qkxmessagebox.h"
#include "qwosessionproperty.h"

#include <QCloseEvent>
#include <QApplication>
#include <QMenu>
#include <QLineEdit>
#include <QTabBar>
#include <QVBoxLayout>
#include <QSplitter>


QWoTermWidgetImpl::QWoTermWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, parent)
    , m_gid(gid)
    , m_tab(tab)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
    m_root = new QSplitter(this);
    m_root->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_root);
    m_termRoot = new QSplitter(this);
    m_termRoot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_root->addWidget(m_termRoot);
    m_root->setStretchFactor(1, 1);
    m_root->setChildrenCollapsible(false);
    m_termRoot->setHandleWidth(1);
    m_termRoot->setOpaqueResize(false);
    m_termRoot->setChildrenCollapsible(false);
    QObject::connect(m_termRoot, SIGNAL(destroyed(QObject*)), this, SLOT(onRootSplitterDestroy()));

    setAutoFillBackground(true);
    QPalette pal;
    QColor clr = Qt::gray;
    pal.setColor(QPalette::Background, clr);
    pal.setColor(QPalette::Window, clr);
    setPalette(pal);

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QWoTermWidgetImpl::~QWoTermWidgetImpl()
{

}

void QWoTermWidgetImpl::joinToVertical(const QString &target)
{
    if(m_terms.isEmpty()) {
        return;
    }
    m_terms.last()->splitWidget(target, QWoUtils::gid(), true);
}

void QWoTermWidgetImpl::joinToHorizontal(int row, const QString &target)
{
    if(m_terms.isEmpty()) {
        return;
    }
    if(row >= m_terms.length() || row < 0) {
        m_terms.last()->splitWidget(target, QWoUtils::gid(), false);
        return;
    }
    m_terms.at(row)->splitWidget(target, QWoUtils::gid(), false);
}

void QWoTermWidgetImpl::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

bool QWoTermWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoTermWidgetImpl::handleTabContextMenu(QMenu *menu)
{
//    if(m_terms.length() > 1) {
//        menu->addAction(tr("Command line input..."), this, SLOT(onCommandInputInSamePage()));
    //    }
}

bool QWoTermWidgetImpl::handleCustomProperties()
{
    if(QWoShowerWidget::handleCustomProperties()) {
        for(int i = 0; i < m_terms.length(); i++) {
            QWidget *w = m_terms.at(i);
            QMetaObject::invokeMethod(w, "initCustom", Qt::QueuedConnection);
        }
    }
    return true;
}

void QWoTermWidgetImpl::updateEnable(bool on)
{
    for(int i = 0; i < m_terms.length(); i++) {
        m_terms[i]->setUpdatesEnabled(on);
    }
}

void QWoTermWidgetImpl::addAssistant(QWidget *w, bool first)
{
    m_root->insertWidget(first ? 0 : -1, w);
    QList<int> ls;
    int width = m_root->width();
    int cnt = m_root->count();
    int left = width / cnt / 2;
    int right = width - left;
    ls << left;
    for(int i = 0; i < cnt - 1; i++) {
        ls << right / (cnt - 1);
    }
    for(int i = 0; i < m_root->count(); i++) {
        m_root->setStretchFactor(i, 1);
    }
    m_root->setSizes(ls);
}

void QWoTermWidgetImpl::onRootSplitterDestroy()
{
    close();
    deleteLater();
}

void QWoTermWidgetImpl::init()
{
    QWoTermWidget *term = createTermWidget(m_target, m_gid, m_termRoot);
    m_termRoot->addWidget(term);
}

QMap<QString, QString> QWoTermWidgetImpl::collectUnsafeCloseMessage()
{
    QMap<QString, QString> all;
    for(int i = 0; i < m_terms.length(); i++) {
        QWoTermWidget *term = m_terms.at(i);
        QList<QString> msgs = term->collectUnsafeCloseMessage();
        if(!msgs.isEmpty()){
            all.insert(term->termName(), msgs.join("\r\n"));
        }
    }
    return all;
}

void QWoTermWidgetImpl::broadcastMessage(int type, QVariant msg)
{
    for(int i = 0; i < m_terms.count(); i++) {
        QWoTermWidget *term = m_terms.at(i);
        if(term) {
           // term->onBroadcastMessage(type, msg);
        }
    }
}

void QWoTermWidgetImpl::addToList(QWoTermWidget *w)
{
    m_terms.append(w);
    resetTabText();
}

void QWoTermWidgetImpl::removeFromList(QWoTermWidget *w)
{
    m_terms.removeOne(w);
    resetTabText();
}

void QWoTermWidgetImpl::resetTabText()
{
    if(m_terms.isEmpty()) {
        return;
    }
    QString title = m_terms.at(0)->target();
    if(m_terms.length() > 1) {
        title.append("*");
    }
    setTabText(title);
}

void QWoTermWidgetImpl::setTabText(const QString &title)
{
    for(int i = 0; i < m_tab->count(); i++){
        QVariant data = m_tab->tabData(i);
        QWoTermWidgetImpl *impl = data.value<QWoTermWidgetImpl*>();
        if(impl == this){
            m_tab->setTabText(i, title);
        }
    }
}

bool QWoTermWidgetImpl::event(QEvent *e)
{
    QEvent::Type t = e->type();
    if(t == QWoEvent::EventType) {
        for(int i = 0; i < m_terms.count(); i++) {
            QWoTermWidget *term = m_terms.at(i);
            QCoreApplication::sendEvent(term, e);
        }
        return false;
    }
    return QWoShowerWidget::event(e);
}
