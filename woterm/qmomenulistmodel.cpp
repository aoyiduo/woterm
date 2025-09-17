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

#include "qmomenulistmodel.h"

#include <QTimer>

#define ITEM_HEIGHT (45)

QMoMenuListModel::QMoMenuListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onLaterReset()));
}

void QMoMenuListModel::remove(int mid)
{

}

void QMoMenuListModel::update(int id, const QString &name, const QIcon &ico)
{
    auto it = std::find_if(m_items.begin(), m_items.end(), [=](const MenuData& md){
       return md.id == id;
    });
    if(it != m_items.end()) {
        MenuData& md = *it;
        md.icon = ico;
        md.name = name;
    }else{
        MenuData md;
        md.id = id;
        md.name = name;
        md.icon = ico;
        m_items.append(md);
    }
    m_timer->start(0);
}

int QMoMenuListModel::itemCount() const
{
    return m_items.length();
}

int QMoMenuListModel::itemsHeight() const
{
    return m_items.length() * ITEM_HEIGHT;
}

int QMoMenuListModel::rowCount(const QModelIndex &parent) const
{
    return m_items.length();
}

int QMoMenuListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QMoMenuListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0 || row >= m_items.length()) {
        return QVariant();
    }
    const MenuData& md = m_items.at(row);
    if(role == Qt::DecorationRole) {
        return md.icon;
    }
    if(role == Qt::DisplayRole) {
        return md.name;
    }
    if(role == Qt::SizeHintRole) {
        return QSize(-1, ITEM_HEIGHT);
    }
    if(role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    if(role == ROLE_MENUID) {
        return md.id;
    }
    return QVariant();
}

QVariant QMoMenuListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

void QMoMenuListModel::onLaterReset()
{
    m_timer->stop();
    beginResetModel();
    endResetModel();
}
