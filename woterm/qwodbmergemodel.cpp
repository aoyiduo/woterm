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

#include "qwodbmergemodel.h"
#include "qwosshconf.h"

#include <QGuiApplication>
#include <QFontMetrics>

QWoDBMergeModel::QWoDBMergeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_font = QGuiApplication::font();
}

void QWoDBMergeModel::setFont(const QFont &ft)
{
    m_font = ft;
}

void QWoDBMergeModel::setData(const MergeInfo &mi)
{
    m_mi = mi;
}

QString QWoDBMergeModel::reset(bool isFull)
{
    m_isFull = isFull;
    QList<QVariantMap> lsvm = m_mi.result(isFull);
    beginResetModel();
    m_datas.clear();
    QFontMetrics fm(m_font);
    m_maxWidth = 0;
    for(auto it = lsvm.cbegin(); it != lsvm.cend(); it++) {
        QVariantMap dm = *it;
        QVariantMap local = dm.value("local").toMap();
        QVariantMap remote = dm.value("remote").toMap();
        QString localLabel = toString(local);
        QString remoteLabel = toString(remote);
        QSize txtSize1 = fm.size(Qt::TextWordWrap, localLabel);
        QSize txtSize2 = fm.size(Qt::TextWordWrap, remoteLabel);
        int txtWidth = qMax<int>(txtSize1.width(), txtSize2.width());
        int txtHeight = qMax<int>(txtSize1.height(), txtSize2.height());
        if(txtWidth > m_maxWidth) {
            m_maxWidth = txtWidth;
        }
        dm.insert("localLabel", localLabel);
        dm.insert("remoteLabel", remoteLabel);
        dm.insert("rowHeight", txtHeight);
        dm.insert("colWidth", txtWidth);
        m_datas.append(dm);
    }
    endResetModel();
    return m_mi.resultInformation();
}

void QWoDBMergeModel::clear()
{
    m_mi.clear();
    reset(false);
}

/*
std::string sql = "update servers set type=@type,host=@host,port=@port,loginName=@loginName,";
sql += "loginPassword=@loginPassword,identityFile=@identityFile,scriptFile=@scriptFile,script=@script,";
sql += "proxyJump=@proxyJump,memo=@memo,property=@property,groupName=@groupName,baudRate=@baudRate,";
sql += "dataBits=@dataBits,parity=@parity,stopBits=@stopBits,flowControl=@flowControl ";
sql += "where name=@name and delFlag=0";
*/

static void qVariantMapMergeToHostInfo(HostInfo& hi, QVariantMap dm) {
    hi.name = dm.value("name").toString();
    hi.type = (EHostType)dm.value("type").toInt();
    hi.host = dm.value("host").toString();
    hi.port = dm.value("port").toInt();
    hi.user = dm.value("loginName").toString();
    hi.password = dm.value("loginPassword").toString();
    hi.identityFile = dm.value("identityFile").toString();
    hi.scriptFile = dm.value("scriptFile").toString();
    hi.script = dm.value("script").toString();
    hi.proxyJump = dm.value("proxyJump").toString();
    hi.memo = dm.value("meno").toString();

    hi.property = dm.value("property").toString();
    hi.group = dm.value("groupName").toString();
    hi.baudRate = dm.value("baudRate").toString();
    hi.dataBits = dm.value("dataBits").toString();
    hi.parity = dm.value("parity").toString();
    hi.stopBits = dm.value("stopBits").toString();
    hi.flowControl = dm.value("flowControl").toString();
}

static HostInfo qVariantMapToHostInfo(QVariantMap dm) {
    HostInfo hi;
    hi.name = dm.value("name").toString();
    hi.type = (EHostType)dm.value("type").toInt();
    hi.host = dm.value("host").toString();
    hi.port = dm.value("port").toInt();
    hi.user = dm.value("loginName").toString();
    hi.password = dm.value("loginPassword").toString();
    hi.identityFile = dm.value("identityFile").toString();
    hi.scriptFile = dm.value("scriptFile").toString();
    hi.script = dm.value("script").toString();
    hi.proxyJump = dm.value("proxyJump").toString();
    hi.memo = dm.value("meno").toString();
    hi.property = dm.value("property").toString();
    hi.group = dm.value("groupName").toString();
    hi.baudRate = dm.value("baudRate").toString();
    hi.dataBits = dm.value("dataBits").toString();
    hi.parity = dm.value("parity").toString();
    hi.stopBits = dm.value("stopBits").toString();
    hi.flowControl = dm.value("flowControl").toString();
    return hi;
}

void QWoDBMergeModel::apply()
{
    // adds
    for(auto it = m_mi.rhave.begin(); it != m_mi.rhave.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "add") {
            dm.insert("mergeAction", "done");
            HostInfo hi = qVariantMapToHostInfo(remote);
            QWoSshConf::instance()->append(hi);
        }
    }

    // remove
    for(auto it = m_mi.remove.begin(); it != m_mi.remove.end(); ) {
        QVariantMap dm = *it;
        QVariantMap local = dm.value("local").toMap();
        QString name = local.value("name").toString();
        QWoSshConf::instance()->removeServer(name);
        it = m_mi.remove.erase(it);
    }

    // replace
    for(auto it = m_mi.replace.begin(); it != m_mi.replace.end(); it++) {
        QVariantMap& dm = *it;
        QVariantMap remote = dm.value("remote").toMap();
        QString action = dm.value("mergeAction").toString();
        if(action == "replace") {
            dm.insert("mergeAction", "done");
            HostInfo hi;
            qVariantMapMergeToHostInfo(hi, remote);
            QWoSshConf::instance()->modify(hi);
        }
    }
}

bool QWoDBMergeModel::runAction(int action, const QModelIndex &idx)
{
    int row = idx.row();
    if(row >= m_datas.length()) {
        return false;
    }
    QVariantMap dm = m_datas.at(row);
    QVariantMap local = dm.value("local").toMap();
    QVariantMap remote = dm.value("remote").toMap();
    if(action == DB_MERGE_ACTION_ADD) {
        QString name = remote.value("name").toString();
        auto it = std::find_if(m_mi.rhave.begin(), m_mi.rhave.end(), [=](const QVariantMap& dm){
            QVariantMap remote = dm.value("remote").toMap();
            QString nameHit = remote.value("name").toString();
            return name == nameHit;
        });
        if(it == m_mi.rhave.end()) {
            return false;
        }
        QVariantMap& dm = *it;
        dm.insert("local", dm.value("remote"));
        dm.insert("mergeAction", "add");
        dm.insert("isSame", true);
    }else if(action == DB_MERGE_ACTION_REMOVE) {
        QString name = local.value("name").toString();
        auto it = std::find_if(m_mi.lhave.begin(), m_mi.lhave.end(), [=](const QVariantMap& dm){
            QVariantMap local = dm.value("local").toMap();
            QString nameHit = local.value("name").toString();
            return name == nameHit;
        });
        if(it == m_mi.lhave.end()) {
            return false;
        }
        m_mi.remove.append(*it);
        m_mi.lhave.erase(it);
    }else if(action == DB_MERGE_ACTION_REPLACE) {
        QString name = remote.value("name").toString();
        auto it = std::find_if(m_mi.replace.begin(), m_mi.replace.end(), [=](const QVariantMap& dm){
            QVariantMap remote = dm.value("remote").toMap();
            QString nameHit = remote.value("name").toString();
            return name == nameHit;
        });
        if(it == m_mi.replace.end()) {
            return false;
        }
        QVariantMap& dm = *it;
        dm.insert("local", dm.value("remote"));
        dm.insert("mergeAction", "replace");
        dm.insert("isSame", true);
    }
    reset(m_isFull);
    return true;
}

int QWoDBMergeModel::rowCount(const QModelIndex &parent) const
{
    return m_datas.length();
}

int QWoDBMergeModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant QWoDBMergeModel::data(const QModelIndex &idx, int role) const
{
    int row = idx.row();
    int col = idx.column();
    if(!idx.isValid() || row >= m_datas.length() || col >= 3) {
        return QVariant();
    }
    QVariantMap dm = m_datas.at(row);
    QString localLabel = dm.value("localLabel").toString();
    QString remoteLabel = dm.value("remoteLabel").toString();
    int colWidth = dm.value("colWidth").toInt();
    int rowHeight = dm.value("rowHeight").toInt();
    bool isSame = dm.value("isSame").toBool();
    colWidth = m_maxWidth + 10;

    if(role == Qt::FontRole){
        return m_font;
    }else if(role == Qt::ToolTipRole) {
        if(col == 0) {
            return localLabel;
        }else if(col == 1) {
            return remoteLabel;
        }
        return QVariant();
    }else if(role == Qt::SizeHintRole) {
        if(colWidth > 240) {
            colWidth = 240;
        }
        if(rowHeight > 180) {
            rowHeight = 180;
        }
        return QSize(colWidth, rowHeight);
    }else if(role == Qt::DisplayRole) {
        if(col == 0) {
            return localLabel;
        }else if(col == 1) {
            return remoteLabel;
        }else if(col == 2) {
            if(localLabel.isEmpty()) {
                return "add";
            }else if(remoteLabel.isEmpty()) {
                return "remove";
            }else if(!isSame) {
                return "replace";
            }
            return QVariant();
        }
    }
    return QVariant();
}

QVariant QWoDBMergeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == 0) {
                return tr("Local");
            }else if(section == 1) {
                return tr("Remote");
            }else if(section == 2){
                return tr("Operate");
            }
        }
    }else{
        if(role == Qt::DisplayRole) {
            if(section < m_datas.length()) {
                return QString::number(section);
            }
        }
    }
    return QVariant();
}
