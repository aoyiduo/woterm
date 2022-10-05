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

#include "qwohosttreemodel.h"
#include "qwosshconf.h"


#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>

QWoHostTreeModel::QWoHostTreeModel(QObject *parent)
    : QAbstractItemModel (parent)
    , m_maxColumn(1)
{
    m_root = new TreeItem;
    m_root->parent = nullptr;
    refreshList();
}

QWoHostTreeModel::~QWoHostTreeModel()
{

}

QWoHostTreeModel *QWoHostTreeModel::instance()
{
    static QWoHostTreeModel *model = new QWoHostTreeModel();
    return model;
}

void QWoHostTreeModel::setMaxColumnCount(int cnt)
{
    m_maxColumn = cnt;
}

int QWoHostTreeModel::widthColumn(const QFont &ft, int i)
{
    QFontMetrics fm(ft);
    QList<HostInfo> his = QWoSshConf::instance()->hostList();
    int maxWidth = 0;
    for(int i = 0; i < his.count(); i++) {
        int w = fm.width(his.at(i).name);
        if(maxWidth < w) {
            maxWidth = w;
        }
    }
    return maxWidth;
}

void QWoHostTreeModel::refreshList()
{
    if(QWoSshConf::instance()->refresh()){
        emit beginResetModel();
        QList<HostInfo> hosts = QWoSshConf::instance()->hostList();
        clear();
        for(int i = 0; i < hosts.count(); i++) {
            HostInfo hi = hosts.at(i);
            QString cat = hi.group;
            if(cat.isEmpty()) {
                cat = "Unknow";
            }
            bool hasAppend = false;
            for(int j = 0; j < m_root->childs.count(); j++) {
                TreeItem *group = m_root->childs.at(j);
                if(group->name == cat) {
                    TreeItem *leaf = new TreeItem;
                    leaf->hi = hi;
                    leaf->parent = group;
                    leaf->name = cat;
                    group->childs.append(leaf);
                    hasAppend = true;
                    break;
                }
            }
            if(!hasAppend) {
                TreeItem *group = new TreeItem;
                group->name = cat;
                group->parent = m_root;
                m_root->childs.append(group);

                TreeItem *leaf = new TreeItem;
                leaf->hi = hi;
                leaf->parent = group;
                leaf->name = cat;
                group->childs.append(leaf);
            }
        }
        emit endResetModel();
    }
}

void QWoHostTreeModel::add(const HostInfo& hi)
{
    QWoSshConf::instance()->append(hi);
    refreshList();
}

bool QWoHostTreeModel::exists(const QString &name)
{
    return QWoSshConf::instance()->exists(name);
}

void QWoHostTreeModel::resetAllProperty(QString v)
{
    QWoSshConf::instance()->resetAllProperty(v);
    refreshList();
}

void QWoHostTreeModel::modify(int idx, const HostInfo &hi)
{
    QWoSshConf::instance()->modify(hi);
    refreshList();
}

void QWoHostTreeModel::append(const HostInfo &hi)
{
    QWoSshConf::instance()->append(hi);
    refreshList();
}

int QWoHostTreeModel::rowCount(const QModelIndex &parent) const
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

QModelIndex QWoHostTreeModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return QAbstractItemModel::sibling(row, column, idx);
}

QVariant QWoHostTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Host");
        case 2:
            return tr("Memo");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant QWoHostTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }
    if (role != Qt::DisplayRole){
        return QVariant();
    }
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if(item == nullptr) {
        return QVariant();
    }
    if(item == m_root) {
        return QVariant("Root");
    }
    if(item->parent == m_root) {
        if(index.column() == 0) {
            return QVariant(item->name);
        }
        return QVariant();
    }
    switch (index.column()) {
    case 0:
        return item->hi.name;
    case 1:
        return item->hi.host;
    case 2:
        return item->hi.memo;
    default:
        return QVariant();
    }
}



bool QWoHostTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

QModelIndex QWoHostTreeModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex QWoHostTreeModel::parent(const QModelIndex &index) const
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

Qt::ItemFlags QWoHostTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()){
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool QWoHostTreeModel::insertRows(int row, int count, const QModelIndex &parent)
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

bool QWoHostTreeModel::removeRows(int row, int count, const QModelIndex &parent)
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

static bool ascendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first < s2.first;
}

static bool decendingLessThan(const QPair<QString, int> &s1, const QPair<QString, int> &s2)
{
    return s1.first > s2.first;
}

void QWoHostTreeModel::sort(int column, Qt::SortOrder order)
{
#if 1
    return QAbstractItemModel::sort(column, order);
#else
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), VerticalSortHint);

     QVector<QPair<QString, HostInfo> > list;
     const int lstCount = m_hosts.count();
     list.reserve(lstCount);
     for (int i = 0; i < lstCount; ++i){
         list.append(QPair<QString, HostInfo>(m_hosts.at(i), i));
     }

     if (order == Qt::AscendingOrder)
         std::sort(list.begin(), list.end(), ascendingLessThan);
     else
         std::sort(list.begin(), list.end(), decendingLessThan);

     m_hosts.clear();
     QVector<int> forwarding(lstCount);
     for (int i = 0; i < lstCount; ++i) {
         m_hosts.append(list.at(i).first);
         forwarding[list.at(i).second] = i;
     }

     QModelIndexList oldList = persistentIndexList();
     QModelIndexList newList;
     const int numOldIndexes = oldList.count();
     newList.reserve(numOldIndexes);
     for (int i = 0; i < numOldIndexes; ++i)
         newList.append(index(forwarding.at(oldList.at(i).row()), 0));
     changePersistentIndexList(oldList, newList);

     emit layoutChanged(QList<QPersistentModelIndex>(), VerticalSortHint);
#endif
}

int QWoHostTreeModel::columnCount(const QModelIndex &parent) const
{
    return m_maxColumn;
}

Qt::DropActions QWoHostTreeModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions();
}

void QWoHostTreeModel::clear()
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
