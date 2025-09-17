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

#include "qwohostlistmodel.h"
#include "qwosshconf.h"

#include <QPair>
#include <QFontMetrics>
#include <QVector>
#include <QDebug>
#include <QPixmap>

Q_GLOBAL_STATIC(QWoHostListModel, glistModel)
QWoHostListModel::QWoHostListModel(QObject *parent)
    : QAbstractListModel (parent)
{
    m_sshIcon = QIcon(QPixmap("../private/skins/black/ssh2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_sftpIcon = QIcon(QPixmap("../private/skins/black/sftp.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_telnetIcon = QIcon(QPixmap("../private/skins/black/telnet.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_rloginIcon = QIcon(QPixmap("../private/skins/black/rlogin.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_mstscIcon = QIcon(QPixmap("../private/skins/black/mstsc2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_vncIcon = QIcon(QPixmap("../private/skins/black/vnc2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_serialIcon = QIcon(QPixmap("../private/skins/black/serialport.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    QMetaObject::invokeMethod(this, "refreshList", Qt::QueuedConnection);

    QObject::connect(QWoSshConf::instance(), SIGNAL(dataReset()), this, SLOT(onDataReset()));
}

QWoHostListModel::~QWoHostListModel()
{

}

QWoHostListModel *QWoHostListModel::instance()
{
    return glistModel;
}

int QWoHostListModel::widthColumn(const QFont &ft, int i)
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

void QWoHostListModel::refreshList()
{
    if(QWoSshConf::instance()->refresh()){
        beginResetModel();
        m_hosts = QWoSshConf::instance()->hostList();
        endResetModel();
    }
}

bool QWoHostListModel::exists(const QString &name)
{
    return QWoSshConf::instance()->exists(name);
}

void QWoHostListModel::resetAllProperty(QString v)
{
    QWoSshConf::instance()->resetAllProperty(v);
    refreshList();
}

void QWoHostListModel::modifyOrAppend(const HostInfo &hi)
{
    if(QWoSshConf::instance()->modifyOrAppend(hi)){
        refreshList();
    }
}

bool QWoHostListModel::qmlRemove(const QString &name)
{
    return QWoSshConf::instance()->removeServer(name);
}

QVariantMap QWoHostListModel::qmlGet(int row)
{
    if(row >= m_hosts.length() || row < 0) {
        return QVariantMap();
    }
    const HostInfo& hi = m_hosts.at(row);
    if(!hi.isValid()) {
        return QVariantMap();
    }
    QVariantMap dm;
    dm.insert("name", hi.name);
    dm.insert("host", hi.host);
    dm.insert("port", QString("%1").arg(hi.port));
    dm.insert("memo", hi.memo);
    dm.insert("user", hi.user);
    dm.insert("password", hi.password);
    dm.insert("identify", hi.identityFile);
    dm.insert("script", hi.script);
    dm.insert("proxyJump", hi.proxyJump);
    if(hi.type == SshWithSftp) {
        dm.insert("type", "SshWithSftp");
        dm.insert("icon", "qrc../private/skins/black/ssh2.png");
        dm.insert("tip", QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group));
    }else if(hi.type == SftpOnly) {
        dm.insert("type", "SftpOnly");
        dm.insert("icon", "qrc../private/skins/black/sftp.png");
        dm.insert("tip", QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group));
    }else if(hi.type == Telnet) {
        dm.insert("type", "Telnet");
        dm.insert("icon", "qrc../private/skins/black/telnet.png");
        dm.insert("tip", QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group));
    }else if(hi.type == RLogin) {
        dm.insert("type", "RLogin");
        dm.insert("icon", "qrc../private/skins/black/rlogin.png");
        dm.insert("tip", QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group));
    }else if(hi.type == SerialPort) {
        dm.insert("type", "RLogin");
        dm.insert("icon", "qrc../private/skins/black/serialport.png");
        dm.insert("tip", QString("Should be remove"));
    }else if(hi.type == Mstsc) {
        dm.insert("type", "RLogin");
        dm.insert("icon", "qrc../private/skins/black/mstsc2.png");
        dm.insert("tip", QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group));
    }else if(hi.type == Vnc) {
        dm.insert("type", "RLogin");
        dm.insert("icon", "qrc../private/skins/black/vnc2.png");
        dm.insert("tip", QString("%1:%2 - %3").arg(hi.host).arg(hi.port).arg(hi.group));
    }else{
        dm.insert("type", "Unknow");
        dm.insert("icon", "qrc../private/skins/black/vnc2.png");
        dm.insert("tip", QString("Unknow"));
    }

    return dm;
}

void QWoHostListModel::onDataReset()
{
    refreshList();
}

int QWoHostListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }    
    return m_hosts.count();
}

QModelIndex QWoHostListModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || row >= m_hosts.count() || row < 0){
        return QModelIndex();
    }

    return createIndex(row, column);
}

QVariant QWoHostListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Host");
        case 2:
            return tr("Type");
        case 3:
            return tr("Memo");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QWoHostListModel::data(const QModelIndex &index, int role) const
{
    //qDebug() << "HostListModel" << role;
    if (index.row() < 0 || index.row() >= m_hosts.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    if(role == Qt::SizeHintRole) {
        return QSize(-1, 25);
    }
    const HostInfo& hi = m_hosts.at(index.row());

    if(role == ROLE_ICON_URL) {
        if(index.column() != 0) {
            return QVariant();
        }
        switch (hi.type) {
        case SshWithSftp:
            return "qrc../private/skins/black/ssh2.png";
        case SftpOnly:
            return "qrc../private/skins/black/sftp.png";
        case Telnet:
            return "qrc../private/skins/black/telnet.png";
        case RLogin:
            return "qrc../private/skins/black/rlogin.png";
        case Mstsc:
            return "qrc../private/skins/black/mstsc2.png";
        case Vnc:
            return "qrc../private/skins/black/vnc2.png";
        case SerialPort:
            return "qrc../private/skins/black/serialport.png";
        }
        return QVariant();
    }
    if(role == ROLE_SHORT_STRING) {
        if(index.column() != 0) {
            return QVariant();
        }
        switch (hi.type) {
        case Vnc:
            return QString("%1:%2 - %3").arg(hi.host).arg(hi.port).arg(hi.group);
        case SerialPort:
            return tr("Should be remove");
        }
        return QString("%1:%2 - %3 - %4").arg(hi.host).arg(hi.port).arg(hi.user).arg(hi.group);
    }
    if(role == ROLE_GROUP_NAME) {
        return hi.group;
    }
    if(role == Qt::DecorationRole) {
        if(index.column() != 0) {
            return QVariant();
        }
        switch (hi.type) {
        case SshWithSftp:
            return m_sshIcon;
        case SftpOnly:
            return m_sftpIcon;
        case Telnet:
            return m_telnetIcon;
        case RLogin:
            return m_rloginIcon;
        case Mstsc:
            return m_mstscIcon;
        case Vnc:
            return m_vncIcon;
        case SerialPort:
            return m_serialIcon;
        }
        return QVariant();
    }
    if(role == Qt::ToolTipRole) {
        QString tip = hi.name;
        if(hi.name != hi.host && !hi.host.isEmpty()) {
            tip.append("-").append(hi.host);
        }
        if(!hi.memo.isEmpty()){
            tip.append("-").append(hi.memo);
        }
        return tip;
    }

    if(role == ROLE_INDEX) {
        return index.row();
    }

    if(role == ROLE_HOSTINFO) {
        QVariant v;
        v.setValue(hi);
        return v;
    }

    if(role == ROLE_TYPE) {
        switch (hi.type) {
        case SftpOnly:
            return QVariant("SftpOnly");
        case SshWithSftp:
            return QVariant("SshWithSftp");
        case Telnet:
            return QVariant("Telnet");
        case RLogin:
            return QVariant("RLogin");
        case Mstsc:
            return QVariant("Mstsc");
        case Vnc:
            return QVariant("Vnc");
        case SerialPort:
            return QVariant("SerialPort");
        }
        return QVariant("Unknow");
    }
    if(role == ROLE_REFILTER) {
        QVariant v;
        QString hp = QString("%1:%2").arg(hi.host).arg(hi.port);
        QString filter=hi.name+hp+hi.memo+hi.name+hi.memo;
        v.setValue(filter);
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
            if(hi.type == SerialPort) {
                return QVariant(QString("%1:%2:%3:%4:%5").arg(hi.baudRate).arg(hi.dataBits).arg(hi.parity).arg(hi.stopBits).arg(hi.flowControl));
            }
            return QVariant(QString("%1:%2").arg(hi.host).arg(hi.port));
        case 2:
            switch (hi.type) {
            case SftpOnly:
                return QVariant("SftpOnly");
            case SshWithSftp:
                return QVariant("SshWithSftp");
            case Telnet:
                return QVariant("Telnet");
            case RLogin:
                return QVariant("RLogin");
            case Mstsc:
                return QVariant("Mstsc");
            case Vnc:
                return QVariant("Vnc");
            case SerialPort:
                return QVariant("SerialPort");
            }
            return QVariant("Unknow");
        case 3:
            return QVariant(hi.memo.simplified());
        }
    }
    return QVariant();
}



bool QWoHostListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoHostListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()){
        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
    }
    return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool QWoHostListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)){
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r){
        m_hosts.insert(row, HostInfo());
    }

    endInsertRows();

    return QAbstractListModel::insertRows(row, count, parent);
}

bool QWoHostListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = m_hosts.begin() + row;
    m_hosts.erase(it, it + count);

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

void QWoHostListModel::sort(int column, Qt::SortOrder order)
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

int QWoHostListModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

Qt::DropActions QWoHostListModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

QHash<int, QByteArray> QWoHostListModel::roleNames() const
{
     QHash<int,QByteArray> roles = QAbstractListModel::roleNames();
     roles.insert(Qt::DisplayRole, "name");
     roles.insert(ROLE_ICON_URL, "iconUrl");
     roles.insert(ROLE_SHORT_STRING, "shortString");
     roles.insert(ROLE_GROUP_NAME, "groupName");
     roles.insert(ROLE_TYPE, "type");
     return roles;
}
