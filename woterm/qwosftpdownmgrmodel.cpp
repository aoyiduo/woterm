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

#include "qwosftpdownmgrmodel.h"

#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>
#include <QIcon>

QWoSftpDownMgrModel::QWoSftpDownMgrModel(QObject *parent)
    : QAbstractListModel (parent)
{
}

QWoSftpDownMgrModel::~QWoSftpDownMgrModel()
{

}

int QWoSftpDownMgrModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }    
    return m_fileInfos.count();
}

QModelIndex QWoSftpDownMgrModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || column != 0 || row >= m_fileInfos.count() || row < 0)
        return QModelIndex();

    return createIndex(row, 0);
}

QVariant QWoSftpDownMgrModel::headerData(int section, Qt::Orientation orientation, int role) const
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
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QWoSftpDownMgrModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_fileInfos.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    const FileInfo& hi = m_fileInfos.at(index.row());

    if(role == Qt::ToolTipRole) {
        QString tip = hi.name;
        return tip;
    }

    if(role == ROLE_INDEX) {
        return index.row();
    }

    if(role == ROLE_FILEINFO) {
        QVariant v;
        v.setValue(hi);
        return v;
    }
    if(role == ROLE_REFILTER) {
        QVariant v;
        v.setValue(QString("%1").arg(hi.name));
        return v;
    }
    if(role == ROLE_FRIENDLY_NAME) {
        QVariant v;
        v.setValue(hi.name);
        return v;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int col = index.column();
        switch (col) {
        case 0:
            return QVariant(hi.name);
        case 1:
            return QVariant(QString("%1:%2"));
        }
    }
    return QVariant();
}



bool QWoSftpDownMgrModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoSftpDownMgrModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()){
        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
    }
    return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool QWoSftpDownMgrModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)){
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

//    for (int r = 0; r < count; ++r){
//        m_hosts.insert(row, HostInfo());
//    }

    endInsertRows();

    return QAbstractListModel::insertRows(row, count, parent);
}

bool QWoSftpDownMgrModel::removeRows(int row, int count, const QModelIndex &parent)
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

void QWoSftpDownMgrModel::sort(int column, Qt::SortOrder order)
{
#if 1
    return QAbstractListModel::sort(column, order);
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

int QWoSftpDownMgrModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Qt::DropActions QWoSftpDownMgrModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

static bool lessThan(const FileInfo &s1, const FileInfo &s2)
{
    static QString type = "dl-";
    int t1 = type.indexOf(s1.type);
    int t2 = type.indexOf(s2.type);
    if(t1 < t2) {
        return true;
    }
    if(t1 > t2) {
        return false;
    }
    return s1.name < s2.name;
}

void QWoSftpDownMgrModel::onDirOpen(const QString &path, const QList<QVariant> &v)
{
    emit beginResetModel();
    m_path = path;
    m_fileInfos.clear();
    for(int i = 0; i < v.length(); i++) {
        QMap<QString, QVariant> mdata = v.at(i).toMap();
        FileInfo fi;
        fi.label = mdata.value("label").toString();
        fi.name = mdata.value("name").toString();
        fi.type = mdata.value("type").toString();
        m_fileInfos.append(fi);
    }
    std::sort(m_fileInfos.begin(), m_fileInfos.end(), lessThan);
    emit endResetModel();
}
