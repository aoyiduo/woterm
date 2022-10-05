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

#include "qwoshortcutmodel.h"
#include "qwosshconf.h"


#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QTreeView>

QWoShortCutModel::QWoShortCutModel(QTreeView *tree, QObject *parent)
    : QAbstractItemModel (parent)
    , m_maxColumn(1)
    , m_tree(tree)
{
    m_root = new TreeItem;
    m_root->parent = nullptr;
}

QWoShortCutModel::~QWoShortCutModel()
{

}

void QWoShortCutModel::setMaxColumnCount(int cnt)
{
    m_maxColumn = cnt;
}

int QWoShortCutModel::widthColumn(const QFont &ft, int idx)
{
    QFontMetrics fm(ft);
    int wmax = 0;
    for(int i = 0; i < m_root->childs.size(); i++) {
        TreeItem *n = m_root->childs.at(i);
        QString desc = idx == 0 ? n->desc : n->key.toString();
        int w = fm.width(n->desc);
        if(w > wmax) {
            wmax = w;
        }
    }
    return wmax;
}

void QWoShortCutModel::append(int kid, QString desc, QKeySequence key)
{
    TreeItem *ti = new TreeItem;
    ti->parent = m_root;
    ti->desc = desc;
    ti->isTitle = false;
    ti->key = key;
    ti->kid = kid;
    m_root->childs.append(ti);
}

void QWoShortCutModel::append(QString title)
{
    TreeItem *ti = new TreeItem;
    ti->parent = m_root;
    ti->desc = title;
    ti->isTitle = true;
    ti->kid = 0;
    m_root->childs.append(ti);
}

QKeySequence QWoShortCutModel::value(int kid)
{
    for(int i = 0; i < m_root->childs.size(); i++) {
        TreeItem *n = m_root->childs.at(i);
        if(!n->isTitle) {
            if(n->kid == kid) {
                return n->key;
            }
        }
    }
    return QKeySequence();
}

QMap<int, QKeySequence> QWoShortCutModel::toMap()
{
    QMap<int, QKeySequence> all;
    for(int i = 0; i < m_root->childs.size(); i++) {
        TreeItem *n = m_root->childs.at(i);
        if(!n->isTitle) {
            all.insert(n->kid, n->key);
        }
    }
    return all;
}

int QWoShortCutModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0){
        return 0;
    }

    if (!parent.isValid()){
        parentItem = m_root;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    return parentItem->childs.count();
}

QModelIndex QWoShortCutModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractItemModel::sibling(row, column, idx);
}

QVariant QWoShortCutModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Desc");
        case 1:
            return tr("Key");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant QWoShortCutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if(item == nullptr) {
        return QVariant();
    }
    if(item == m_root) {
        return QVariant("Root");
    }
    if(item->isTitle) {
        if(role == Qt::SizeHintRole) {
            QFontMetrics fm(m_tree->font());
            return QSize(-1, fm.height() * 1.5);
        }
        if(role == Qt::BackgroundColorRole||role == Qt::BackgroundRole){
            return QBrush(Qt::lightGray);
        }
        if(role == Qt::TextColorRole || role == Qt::ForegroundRole){
            return QBrush(Qt::black);
        }

        if(role == Qt::DisplayRole && index.column() == 0){
            return item->desc;
        }
        return QVariant();
    }
    if(role == Qt::SizeHintRole) {
        QFontMetrics fm(m_tree->font());
        if(index.column() == 0) {
            return QSize(-1, fm.height() * 1.5);
        }else if(index.column() == 1){
            return QSize(-1, fm.height() * 1.5);
        }
        return QSize(200, 50);
    }
    if(role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item->desc;
        case 1:
            return item->key;
        default:
            return QVariant();
        }
    }
    if(role == Qt::EditRole) {
        switch (index.column()) {
        case 1:
            return item->key;
        default:
            return QVariant();
        }
    }
    return QVariant();
}



bool QWoShortCutModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()){
        return false;
    }
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if(role == Qt::EditRole) {
        if(index.column() == 1) {
            item->key = value.value<QKeySequence>();
            return true;
        }
    }
    return false;
}

QModelIndex QWoShortCutModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)){
        return QModelIndex();
    }

    TreeItem *parentItem;

    if (!parent.isValid()){
        parentItem = m_root;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    TreeItem *childItem = parentItem->childs[row];
    if (childItem){
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex QWoShortCutModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()){
        return QModelIndex();
    }
    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent;
    if (parentItem == m_root) {
        return QModelIndex();
    }
    int row = parentItem->parent->childs.indexOf(parentItem);
    return createIndex(row, 0, parentItem);
}

Qt::ItemFlags QWoShortCutModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()){
        return Qt::NoItemFlags;
    }
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if(item == nullptr) {
        return Qt::NoItemFlags;
    }
    if(item == m_root) {
        return Qt::NoItemFlags;
    }
    if(item->isTitle) {
        return Qt::NoItemFlags;
    }
    if(index.column() == 1) {
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

bool QWoShortCutModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)){
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

//    for (int r = 0; r < count; ++r){
//        m_hosts.insert(row, HostInfo());
//    }

    endInsertRows();

    return QAbstractItemModel::insertRows(row, count, parent);
}

bool QWoShortCutModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

//    const auto it = m_hosts.begin() + row;
//    m_hosts.erase(it, it + count);

    endRemoveRows();

    return true;
}

void QWoShortCutModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractItemModel::sort(column, order);
}

int QWoShortCutModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

Qt::DropActions QWoShortCutModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions();
}

void QWoShortCutModel::clear()
{
    if(m_root != nullptr) {
        for(int i = 0; i < m_root->childs.count(); i++) {
            TreeItem *group = m_root->childs.at(i);
            if(group == nullptr) {
                for(int j = 0; j < group->childs.count(); j++) {
                    TreeItem *leaf = group->childs.at(j);
                    delete leaf;
                }
                group->childs.clear();
            }
            delete group;
        }
        m_root->childs.clear();
    }
}
