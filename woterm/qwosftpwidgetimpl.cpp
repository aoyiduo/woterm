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

#include "qwosftpwidgetimpl.h"
#include "qwosftpwidget.h"
#include "qwoglobal.h"
#include "qwoshower.h"
#include "qwosshconf.h"
#include "qwomainwindow.h"
#include "qwoevent.h"
#include "qwocommandlineinput.h"

#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QTabBar>
#include <QVBoxLayout>
#include <QSplitter>


QWoSftpWidgetImpl::QWoSftpWidgetImpl(const QString& target, int gid, QTabBar *tab, QWidget *parent)
    : QWoShowerWidget(target, parent)
    , m_gid(gid)
    , m_tab(tab)
{
    m_sftp = new QWoSftpWidget(target, gid, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_sftp);
    setAutoFillBackground(true);
    QPalette pal;
    QColor clr = Qt::gray;
    pal.setColor(QPalette::Background, clr);
    pal.setColor(QPalette::Window, clr);
    setPalette(pal);

    QObject::connect(m_sftp, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyReady()));
}

QWoSftpWidgetImpl::~QWoSftpWidgetImpl()
{

}

void QWoSftpWidgetImpl::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    if(event->isAccepted()) {
        return;
    }
    QWidget::closeEvent(event);
}

bool QWoSftpWidgetImpl::handleTabMouseEvent(QMouseEvent *ev)
{
    return false;
}

void QWoSftpWidgetImpl::handleTabContextMenu(QMenu *menu)
{
    menu->addAction(tr("New Session Multiplex"), this, SLOT(onNewSessionMultplex()));
}

void QWoSftpWidgetImpl::updateEnable(bool on)
{

}

void QWoSftpWidgetImpl::onDestroyReady()
{
    close();
    deleteLater();
}

void QWoSftpWidgetImpl::onNewSessionMultplex()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSftp(m_target, m_gid);
}

QMap<QString, QString> QWoSftpWidgetImpl::collectUnsafeCloseMessage()
{
    QMap<QString, QString> all;

    return all;
}

void QWoSftpWidgetImpl::setTabText(const QString &title)
{
    for(int i = 0; i < m_tab->count(); i++){
        QVariant data = m_tab->tabData(i);
        QWoSftpWidgetImpl *impl = data.value<QWoSftpWidgetImpl*>();
        if(impl == this){
            m_tab->setTabText(i, title);
        }
    }
}
