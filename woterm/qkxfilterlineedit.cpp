/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxfilterlineedit.h"

#include "qwoglobal.h"

#include "qkxfilterlistview.h"
#include "qkxbuttonassist.h"
#include "qwosshconf.h"

#include <QKeyEvent>
#include <QListView>
#include <QDebug>
#include <QDateTime>
#include <QToolButton>
#include <QIcon>

QKxFilterLineEdit::QKxFilterLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    m_listView = new QKxFilterListView(this, topLevelWidget());
    QObject::connect(m_listView, SIGNAL(itemClicked(QString,int)), this, SLOT(onListViewItemClicked(QString,int)));
    QKxButtonAssist *assist = new QKxButtonAssist("../private/skins/black/add2.png", this);
    assist->append("../private/skins/black/connect.png", true);
    QObject::connect(assist, SIGNAL(clicked(int)), this, SLOT(onAssistButtonClicked(int)));
    QObject::connect(this, SIGNAL(returnPressed()), this, SLOT(onAssistReturnPressed()));

    QObject::connect(this, &QLineEdit::textChanged, this, [=](const QString& name){
        QToolButton *btn = assist->button(1);
        if(btn == nullptr) {
            return ;
        }
        bool has = QWoSshConf::instance()->exists(name);
        btn->setIcon(has ? QIcon("../private/skins/black/edit.png") : QIcon("../private/skins/black/add2.png"));
    });
}

QKxFilterLineEdit::~QKxFilterLineEdit()
{

}

void QKxFilterLineEdit::onListViewItemClicked(const QString &name, int type)
{
    m_listView->enableFilter(false);
    setText(name);
    QMetaObject::invokeMethod(m_listView, "enableFilter", Qt::QueuedConnection, Q_ARG(bool, true));
    emit targetArrived(name, type);
}

void QKxFilterLineEdit::onAssistButtonClicked(int idx)
{
    if(idx == 0) {
        // add
        QString name = text();
        bool has = QWoSshConf::instance()->exists(name);
        if(has) {
            emit editArrived(name);
        }else{
            emit createArrived(name);
        }
    }else {
        // connect
        onAssistReturnPressed();
    }
}

void QKxFilterLineEdit::onAssistReturnPressed()
{
    if(!m_listView->isVisible()) {
        QString name = text();
        if(QWoSshConf::instance()->exists(name)) {
            const HostInfo& hi = QWoSshConf::instance()->find(name);
            if(hi.type == SshWithSftp) {
                emit targetArrived(hi.name, EOT_SSH);
            }else if(hi.type == SftpOnly) {
                emit targetArrived(hi.name, EOT_SFTP);
            }else if(hi.type == Telnet) {
                emit targetArrived(hi.name, EOT_TELNET);
            }else if(hi.type == RLogin) {
                emit targetArrived(hi.name, EOT_RLOGIN);
            }else if(hi.type == Mstsc) {
                emit targetArrived(hi.name, EOT_MSTSC);
            }else if(hi.type == Vnc) {
                emit targetArrived(hi.name, EOT_VNC);
            }
        }else{
            emit createArrived(text());
        }
        return;
    }
    QModelIndex idx = m_listView->listView()->currentIndex();
    if(!idx.isValid()) {
        emit createArrived(text());
        return;
    }
    const HostInfo& hi = idx.data(ROLE_HOSTINFO).value<HostInfo>();
    if(hi.type == SshWithSftp) {
        onListViewItemClicked(hi.name, EOT_SSH);
    }else if(hi.type == SftpOnly) {
        onListViewItemClicked(hi.name, EOT_SFTP);
    }else if(hi.type == Telnet) {
        onListViewItemClicked(hi.name, EOT_TELNET);
    }else if(hi.type == RLogin) {
        onListViewItemClicked(hi.name, EOT_RLOGIN);
    }else if(hi.type == Mstsc) {
        onListViewItemClicked(hi.name, EOT_MSTSC);
    }else if(hi.type == Vnc) {
        onListViewItemClicked(hi.name, EOT_VNC);
    }
}

void QKxFilterLineEdit::keyPressEvent(QKeyEvent *ev)
{
    QLineEdit::keyPressEvent(ev);
    int key = ev->key();

    QListView *view = m_listView->listView();
    QModelIndex idx = view->currentIndex();
    int row = idx.row();
    if(key == Qt::Key_Up) {
        idx = idx.sibling(row-1, 0);
        if(!idx.isValid()) {
            return;
        }
        view->setCurrentIndex(idx);
    }else if(key == Qt::Key_Down) {
        idx = idx.sibling(row+1, 0);
        if(!idx.isValid()) {
            return;
        }
        view->setCurrentIndex(idx);
    }else if(key == Qt::Key_Escape) {
        m_listView->hide();
    }
}
