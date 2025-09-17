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

#include "qwohostsimplelist.h"
#include "ui_qwohostlist.h"
#include "qwohostlistmodel.h"
#include "qwosshconf.h"

#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QDebug>

QWoHostSimpleList::QWoHostSimpleList(EHostType type, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoHostList)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("jump list"));
    QStringList names = QWoSshConf::instance()->hostNameList(type);
    m_model = new QStringListModel(names, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui->hostList->setModel(m_proxyModel);

    QObject::connect(ui->rxfind, SIGNAL(textChanged(QString)), this, SLOT(onEditTextChanged(QString)));
    QObject::connect(ui->hostList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onListItemDoubleClicked(QModelIndex)));
    QObject::connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(onOpenSelectSessions()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    ui->rxfind->setReadOnly(false);
    ui->rxfind->clear();
}

QWoHostSimpleList::~QWoHostSimpleList()
{
    if(m_proxyModel) {
        delete m_proxyModel;
    }
    delete ui;
}

bool QWoHostSimpleList::result(HostInfo *phi)
{
    if(m_result.isEmpty()) {
        return false;
    }
    *phi =QWoSshConf::instance()->find(m_result);
    return true;
}

void QWoHostSimpleList::onEditTextChanged(const QString &txt)
{
    QStringList sets = txt.split(' ');
    for(QStringList::iterator iter = sets.begin(); iter != sets.end(); ) {
        if(*iter == "") {
            iter = sets.erase(iter);
        }else{
            iter++;
        }
    }

    QRegExp regex(sets.join(".*"), Qt::CaseInsensitive);
    regex.setPatternSyntax(QRegExp::RegExp2);
    m_proxyModel->setFilterRegExp(regex);
}

void QWoHostSimpleList::onListItemDoubleClicked(const QModelIndex &item)
{
    QString name = item.data().toString();
    if(name == "") {
        return;
    }
    m_result = name;
    done(QDialog::Accepted+1);
}

void QWoHostSimpleList::onOpenSelectSessions()
{
    QModelIndex item = ui->hostList->currentIndex();
    m_result = item.data().toString();
    done(QDialog::Accepted+1);
}
