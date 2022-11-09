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

Q_GLOBAL_STATIC(QWoHostTreeModel, gtreeModel)

QWoHostTreeModel::QWoHostTreeModel(QObject *parent)
    : QStandardItemModel(parent)
{
    m_sshIcon = QIcon(QPixmap(":/woterm/resource/skin/ssh2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_sftpIcon = QIcon(QPixmap(":/woterm/resource/skin/sftp.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_telnetIcon = QIcon(QPixmap(":/woterm/resource/skin/telnet.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_rloginIcon = QIcon(QPixmap(":/woterm/resource/skin/rlogin.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_mstscIcon = QIcon(QPixmap(":/woterm/resource/skin/mstsc2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_vncIcon = QIcon(QPixmap(":/woterm/resource/skin/vnc2.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_serialIcon = QIcon(QPixmap(":/woterm/resource/skin/serialport.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_folderOpenIcon = QIcon(QPixmap(":/woterm/resource/skin/dirs.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_folderCloseIcon = QIcon(QPixmap(":/woterm/resource/skin/dir_close.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));

    QMetaObject::invokeMethod(this, "refreshList", Qt::QueuedConnection);
}

QWoHostTreeModel::~QWoHostTreeModel()
{

}

QWoHostTreeModel *QWoHostTreeModel::instance()
{
    return gtreeModel;
}

void QWoHostTreeModel::refreshList()
{
    if(QWoSshConf::instance()->refresh()){
        clear();
        beginResetModel();
        QList<HostInfo> hosts = QWoSshConf::instance()->hostList();
        QList<GroupInfo> groups = QWoSshConf::instance()->groupList();
        for(int i = 0; i < groups.length(); i++) {
            const GroupInfo& gi = groups.at(i);
            QStandardItem *group = new QStandardItem(gi.name);
            group->setData(QVariant::fromValue<GroupInfo>(gi), ROLE_GROUP);
            int count = 0;
            for(int j = 0; j < hosts.length(); j++) {
                const HostInfo& hi = hosts.at(j);
                if(gi.name == hi.group) {
                    QList<QStandardItem*> cols;
                    {
                        QStandardItem *item = new QStandardItem(hi.name);
                        item->setData(QVariant::fromValue<HostInfo>(hi), ROLE_HOSTINFO);
                        cols.append(item);
                    }
                    {
                        QString hp;
                        if(hi.type == SerialPort) {
                            hp = QString("%1:%2:%3:%4:%5").arg(hi.baudRate).arg(hi.dataBits).arg(hi.parity).arg(hi.stopBits).arg(hi.flowControl);
                        }else{
                            hp = QString("%1:%2").arg(hi.host).arg(hi.port);
                        }
                        cols.append(new QStandardItem(hp));
                    }
                    {
                        QString type;
                        switch (hi.type) {
                        case SftpOnly:
                            type = "SftpOnly";
                            break;
                        case SshWithSftp:
                            type = "SshWithSftp";
                            break;
                        case Telnet:
                            type = "Telnet";
                            break;
                        case RLogin:
                            type = "RLogin";
                            break;
                        case Mstsc:
                            type = "Mstsc";
                            break;
                        case Vnc:
                            type = "Vnc";
                            break;
                        case SerialPort:
                            type = "SerialPort";
                            break;
                        default:
                            type = "Unknow";
                            break;
                        }
                        cols.append(new QStandardItem(type));
                    }
                    {
                        cols.append(new QStandardItem(hi.memo.simplified()));
                    }
                    group->appendRow(cols);
                    count++;
                }
            }
            group->setText(gi.name + QString("[%1]").arg(count));
            appendRow(group);
        }
        endResetModel();
    }
    QStringList labels;
    labels << tr("Name") <<  tr("Host") << tr("Type") << tr("Memo");
    setHorizontalHeaderLabels(labels);
}

QVariant QWoHostTreeModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::SizeHintRole) {
        return QSize(-1, 25);
    }
    if(role == ROLE_REFILTER) {
        QVariant v = QStandardItemModel::data(index, ROLE_HOSTINFO);
        if(!v.isValid()) {
            return v;
        }
        const HostInfo& hi = v.value<HostInfo>();
        QString hp = QString("%1:%2").arg(hi.host).arg(hi.port);
        QString filter=hi.name+hp+hi.memo+hi.name+hi.memo;
        v.setValue(filter);
        return v;
    }else if(role == Qt::DecorationRole) {
        if(index.column() != 0) {
            return QVariant();
        }
        QVariant group = QStandardItemModel::data(index, ROLE_GROUP);
        if(group.isValid()) {
            QVariant state = QStandardItemModel::data(index, ROLE_GROUP_STATE);
            if(!state.isValid()) {
                return QVariant();
            }
            if(state.toBool()) {
                if(hasChildren(index)) {
                    return m_folderOpenIcon;
                }
            }
            return m_folderCloseIcon;
        }
        QVariant v = QStandardItemModel::data(index, ROLE_HOSTINFO);
        if(!v.isValid()) {
            return v;
        }
        const HostInfo& hi = v.value<HostInfo>();
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
        default:
            return QVariant();
        }
        return QVariant();
    }
    return QStandardItemModel::data(index, role);
}
