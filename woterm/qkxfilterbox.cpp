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

#include "qkxfilterbox.h"
#include "qwohostlistmodel.h"

#include <QSortFilterProxyModel>
#include <QLineEdit>

QKxFilterBox::QKxFilterBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(QWoHostListModel::instance());
    m_proxyModel->setRecursiveFilteringEnabled(true);
    setModel(m_proxyModel);
    m_input = lineEdit();
    QObject::connect(m_input, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    QObject::connect(m_input, SIGNAL(textChanged(QString)), this, SLOT(onEditTextChanged(QString)));
}

QKxFilterBox::~QKxFilterBox()
{

}

void QKxFilterBox::onEditReturnPressed()
{

}

void QKxFilterBox::onEditTextChanged(const QString &txt)
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
    m_proxyModel->setFilterRole(ROLE_REFILTER);
}
