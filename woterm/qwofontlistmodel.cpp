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

#include "qwofontlistmodel.h"

#include "qkxutils.h"
#include "qwoglobal.h"

#include <QVariant>
#include <QSize>

QWoFontListModel::QWoFontListModel(QObject *parent)
    : QStringListModel(parent)
{
    reload();
}

QWoFontListModel::~QWoFontListModel()
{

}

void QWoFontListModel::reload()
{
    m_fontsImport = QKxUtils::customFontFamilies();
    m_fontsSystem = QKxUtils::systemFontFamilies();
    setStringList(m_fontsImport + m_fontsSystem);
}

QVariant QWoFontListModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::SizeHintRole) {
        return QSize(-1, 20);
    }
    if(role == ROLE_CUSTOM_FONT) {
        int row = index.row();
        return row >= 0 && row < m_fontsImport.length();
    }
    return QStringListModel::data(index, role);
}

