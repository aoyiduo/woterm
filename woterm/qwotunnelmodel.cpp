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

#include "qwotunnelmodel.h"

#include "qwosetting.h"
#include "qwotunnelserver.h"

#include <QDataStream>
#include <QDateTime>
#include <QSize>
#include <QGuiApplication>
#include <QFontMetrics>

#define ITEM_HEIGHT (50)
#define ICON_SIZE   (36)

QWoTunnelModel::QWoTunnelModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
    m_font = QGuiApplication::font();
    m_icoRunning = QIcon("../private/skins/black/transfer.png");
}

QWoTunnelModel::~QWoTunnelModel()
{

}

bool QWoTunnelModel::add(const TunnelInfo &_ti)
{
    TunnelInfo ti = _ti;
    ti.id = QDateTime::currentMSecsSinceEpoch();
    beginResetModel();
    m_tunnelInfos.append(ti);
    endResetModel();
    save();
    return true;
}

void QWoTunnelModel::modify(const TunnelInfo &ti)
{
    auto it = std::find_if(m_tunnelInfos.begin(), m_tunnelInfos.end(), [=](const TunnelInfo& hit){
        return ti.id == hit.id;
    });
    if(it != m_tunnelInfos.end()) {
        TunnelInfo& hit = *it;
        beginResetModel();
        hit = ti;
        endResetModel();
        save();
    }
}

void QWoTunnelModel::remove(qint64 id)
{
    auto it = std::find_if(m_tunnelInfos.begin(), m_tunnelInfos.end(), [=](const TunnelInfo& hit){
        return id == hit.id;
    });
    if(it != m_tunnelInfos.end()) {
        beginResetModel();
        m_tunnelInfos.erase(it);
        endResetModel();
        save();
    }
}

int QWoTunnelModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_tunnelInfos.count();
}

QVariant QWoTunnelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Type");        
        case 2:
            return tr("Address");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QWoTunnelModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_tunnelInfos.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    if(role == Qt::FontRole) {
        return m_font;
    }
    const TunnelInfo& ti = m_tunnelInfos.at(index.row());
    if(role == ROLE_INDEX) {
        return QVariant::fromValue<TunnelInfo>(ti);
    }
    if(role == Qt::DecorationRole) {
        if(index.column() == 0) {
            if(QWoTunnelFactory::instance()->isRunning(ti.id)) {
                return m_icoRunning;
            }
        }
        return QVariant();
    }
    if(role == Qt::SizeHintRole) {
        if(index.column() == 0) {
            QFontMetrics fm(m_font);
            int w = fm.width(ti.sessionName) + ICON_SIZE;
            return QSize(w, ITEM_HEIGHT);
        }
        return QSize(-1, ITEM_HEIGHT);
    }
    if(role == Qt::DisplayRole) {
        if(index.column() == 0) {
            return ti.sessionName;
        }else if(index.column() == 1) {
            return ti.isPortmapping ? tr("Port mapping") : tr("Socks4/5");
        }else if(index.column() == 2) {
            QString out = tr("Local:");
            out += QString("%1:%2").arg(ti.hostLocal).arg(ti.portLocal);
            out += "\r\n";
            out += tr("Remote:");
            if(ti.isPortmapping) {
                out += QString("%1:%2").arg(ti.hostRemote).arg(ti.portRemote);
            }else{
                out += " - ";
            }
            return out;
        }
    }
    return QVariant();
}



bool QWoTunnelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoTunnelModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

bool QWoTunnelModel::insertRows(int row, int count, const QModelIndex &parent)
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

bool QWoTunnelModel::removeRows(int row, int count, const QModelIndex &parent)
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


void QWoTunnelModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QWoTunnelModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

Qt::DropActions QWoTunnelModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

void QWoTunnelModel::load()
{
    QByteArray buf = QWoSetting::value("tunnelList").toByteArray();
    QDataStream ds(buf);

    beginResetModel();
    int cnt;
    ds >> cnt;
    m_tunnelInfos.clear();
    for(int i = 0; i < cnt; i++) {
        TunnelInfo ti;
        ds >> ti.id >> ti.sessionName >> ti.isPortmapping >> ti.hostRemote >> ti.portRemote >> ti.hostLocal >> ti.portLocal;
        m_tunnelInfos.append(ti);
    }
    endResetModel();
}

void QWoTunnelModel::save()
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << int(m_tunnelInfos.size());
    for(auto it = m_tunnelInfos.begin(); it != m_tunnelInfos.end(); it++) {
        TunnelInfo ti = *it;
        ds << ti.id << ti.sessionName << ti.isPortmapping << ti.hostRemote << ti.portRemote << ti.hostLocal << ti.portLocal;
    }
    QWoSetting::setValue("tunnelList", buf);
}
