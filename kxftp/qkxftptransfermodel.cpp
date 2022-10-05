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

#include "qkxftptransfermodel.h"
#include "qkxftprequest.h"
#include "qkxftpdownload.h"
#include "qkxftpupload.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QDir>

#define ICON_SIZE      (32)

static QDataStream &operator<< (QDataStream &ds, const QKxFtpTransferModel::TaskInfo & info) {
    int status = info.status;
    ds << info.name << info.isUpload << info.localFile << info.remoteFile << info.percent << info.fileSize << status << info.timeLast;
    //qDebug() << "save Item:" << info.deviceId << info.localFile << info.remoteFile << info.status;
    return ds;
}

static QDataStream &operator>> (QDataStream &ds, QKxFtpTransferModel::TaskInfo &info) {
    int status;
    ds >> info.name >> info.isUpload >> info.localFile >> info.remoteFile >> info.percent >> info.fileSize >> status >> info.timeLast;
    info.status = (QKxFtpTransferModel::ETaskStatus)status;
    //qDebug() << "load Item:" << info.deviceId << info.localFile << info.remoteFile << info.status;
     return ds;
}

static bool lessThan(const QKxFtpTransferModel::TaskInfo &s1, const QKxFtpTransferModel::TaskInfo &s2) {
    return s1.timeLast > s2.timeLast;
}

QKxFtpTransferModel::QKxFtpTransferModel(const QString& path, const QString& name, QObject *parent)
    : QAbstractListModel(parent)
    , m_pathTask(path)
    , m_name(name)
    , m_transferIndex(-1)
{
    m_hdrName << tr("Type") << tr("Status") << tr("Progress") << tr("File Size") << tr("Local File") << tr("Remote File");
    m_iconUpload = QIcon(QPixmap(":/kxftp/resource/skin/upload.png").scaled(ICON_SIZE,ICON_SIZE));
    m_iconDown = QIcon(QPixmap(":/kxftp/resource/skin/download.png").scaled(ICON_SIZE,ICON_SIZE));
    m_iconReady = QIcon(QPixmap(":/kxftp/resource/skin/time.png").scaled(ICON_SIZE,ICON_SIZE));
    m_iconPause = QIcon(QPixmap(":/kxftp/resource/skin/pause.png").scaled(ICON_SIZE,ICON_SIZE));
    m_iconTransfer = QIcon(QPixmap(":/kxftp/resource/skin/transfer.png").scaled(ICON_SIZE,ICON_SIZE));
    m_iconFinish = QIcon(QPixmap(":/kxftp/resource/skin/right.png").scaled(ICON_SIZE,ICON_SIZE));

    QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
}

QKxFtpTransferModel::~QKxFtpTransferModel()
{

}

void QKxFtpTransferModel::setFtpRequest(QKxFtpRequest *ftp)
{
    m_ftpRequest = ftp;
}

void QKxFtpTransferModel::addTask(const QList<QString> &files, const QString& localPath, const QString &remotePath, bool isUpload)
{
    emit beginResetModel();
    for(int i = 0; i < files.length(); i++) {
        QString localFile = QDir::cleanPath(localPath + "/" + files.at(i));
        QString remoteFile = QDir::cleanPath(remotePath + "/" + files.at(i));
        if(hasTask(localFile, remoteFile)) {
            continue;
        }
        TaskInfo ti;
        ti.name = m_name;
        ti.localFile = localFile;
        ti.remoteFile = remoteFile;
        ti.timeLast = QDateTime::currentSecsSinceEpoch();
        ti.isUpload = isUpload;
        m_tasks.append(ti);
    }
    emit endResetModel();
    save();
}

void QKxFtpTransferModel::start(const QModelIndex &idx, bool fromZero)
{
    int id_hit = -1;
    int row = idx.row();
    TaskInfo& ti = m_tasks[row];
    if(ti.status == ETS_Pause || ti.status == ETS_Ready) {
        ti.status = ETS_Ready;
        id_hit = row;
    }
    if(m_transferIndex >= 0) {
        return;
    }
    if(id_hit < 0) {
        id_hit = first(ETS_Ready);
    }
    startTransfer(id_hit, fromZero);
}

void QKxFtpTransferModel::restart(const QModelIndex &idx)
{
    stopTransfer();
    int row = idx.row();
    TaskInfo& ti = m_tasks[row];
    if(!ti.isUpload){
       QFile::remove(ti.localFile);
    }
    ti.status = ETS_Ready;
    ti.percent = 0;
    start(idx, true);
}

void QKxFtpTransferModel::startAll()
{
    int id_hit = -1;
    for(int i = 0; i < m_tasks.length(); i++) {
        TaskInfo& ti = m_tasks[i];
        if(ti.status == ETS_Pause) {
            ti.status = ETS_Ready;
            if(id_hit < 0) {
                id_hit = i;
            }
        }
    }
    if(m_transferIndex >= 0) {
        return;
    }
    startTransfer(id_hit);
}

void QKxFtpTransferModel::stop(const QModelIndex &idx)
{
    int id_hit = -1;
    int row = idx.row();
    TaskInfo& ti = m_tasks[row];
    if(m_download) {
        m_download->stop();
    }
    if(m_upload) {
        m_upload->stop();
    }
    if(ti.status == ETS_Transfer) {
        ti.status = ETS_Pause;
        id_hit = row;
        stopTransfer();
        int i = next(row+1, ETS_Ready);
        if(i < 0) {
            i = first(ETS_Ready);
        }
        if(i != id_hit) {
            startTransfer(i);
        }
    }else if(ti.status == ETS_Ready){
        ti.status = ETS_Pause;
    }
}

void QKxFtpTransferModel::stopAll()
{
    for(int i = 0; i < m_tasks.length(); i++) {
        TaskInfo& ti = m_tasks[i];
        if(ti.status == ETS_Ready) {
            ti.status = ETS_Pause;
        }else if(ti.status == ETS_Transfer) {
            ti.status = ETS_Pause;
            stopTransfer();
        }
    }
}

void QKxFtpTransferModel::remove(const QModelIndex &idx)
{
    int row = idx.row();
    if(row < 0 || row >= m_tasks.length()) {
        return;
    }
    if(row == m_transferIndex) {
        stopTransfer();
    }
    removeRow(row);
    int i = next(row, ETS_Ready);
    if(i < 0) {
        i = first(ETS_Ready);
    }
    if(i >= 0) {
        startTransfer(i);
    }
    save();
}

void QKxFtpTransferModel::removeAll()
{
    stopTransfer();
    removeRows(0, m_tasks.length());
    save();
}

int QKxFtpTransferModel::taskCount()
{
    return m_tasks.length();
}

int QKxFtpTransferModel::startCount()
{
    int cnt = 0;
    for(int i = 0; i < m_tasks.length(); i++) {
        const TaskInfo& ti = m_tasks.at(i);
        if(ti.status == ETS_Ready || ti.status == ETS_Transfer) {
            cnt++;
        }
    }
    return cnt;
}

int QKxFtpTransferModel::stopCount()
{
    int cnt = 0;
    for(int i = 0; i < m_tasks.length(); i++) {
        const TaskInfo& ti = m_tasks.at(i);
        if(ti.status == ETS_Pause) {
            cnt++;
        }
    }
    return cnt;
}

int QKxFtpTransferModel::first(QKxFtpTransferModel::ETaskStatus status)
{
    if(status == ETS_Transfer) {
        return m_transferIndex;
    }
    for(int i = 0; i < m_tasks.length(); i++) {
        const TaskInfo& ti = m_tasks.at(i);
        if(ti.status == status) {
            return i;
        }
    }
    return -1;
}

int QKxFtpTransferModel::next(int idx, ETaskStatus status)
{
    if(status == ETS_Transfer) {
        return m_transferIndex;
    }
    for(int i = idx; i < m_tasks.length(); i++) {
        const TaskInfo& ti = m_tasks.at(i);
        if(ti.status == status) {
            return i;
        }
    }
    return -1;
}

QKxFtpTransferModel::ETaskStatus QKxFtpTransferModel::taskStatus(const QModelIndex &idx)
{
    int row = idx.row();
    if(row < 0 || row >= m_tasks.length()) {
        return ETS_Ready;
    }
    const TaskInfo& ti = m_tasks.at(row);
    return ti.status;
}

int QKxFtpTransferModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_tasks.count();
}

QModelIndex QKxFtpTransferModel::sibling(int row, int column, const QModelIndex &idx) const
{    
    return QAbstractListModel::sibling(row, column, idx);
}

QVariant QKxFtpTransferModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        return m_hdrName.at(section);
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QKxFtpTransferModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_tasks.size()) {
        return QVariant();
    }
    if (!index.isValid()){
        return QVariant();
    }
    if(role == Qt::SizeHintRole) {
        return QSize(ICON_SIZE, ICON_SIZE);
    }
    const TaskInfo& ti = m_tasks.at(index.row());
    if(role == Qt::DecorationRole) {
        if(index.column() == 0) {
            if(ti.isUpload) {
                return m_iconUpload;
            }
            return m_iconDown;
        }else if(index.column() == 1) {
            switch (ti.status) {
            case ETS_Pause:
                return m_iconPause;
            case ETS_Transfer:
                return m_iconTransfer;
            case ETS_Finish:
                return m_iconFinish;
            }
            return m_iconReady;
        }
        return QVariant();
    }else if(role == Qt::DisplayRole) {
        if(index.column() == 2) {
            return ti.percent;
        }else if(index.column() == 3) {
            return ti.fileSize;
        }else if(index.column() == 4) {
            return ti.localFile;
        }else if(index.column() == 5) {
            return ti.remoteFile;
        }
        return QVariant();
    }
    return QVariant();
}

bool QKxFtpTransferModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QKxFtpTransferModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index);
    //return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool QKxFtpTransferModel::insertRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "insertRow";
    return false;
}

bool QKxFtpTransferModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = m_tasks.begin() + row;
    m_tasks.erase(it, it + count);

    endRemoveRows();

    return true;
}

void QKxFtpTransferModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QKxFtpTransferModel::columnCount(const QModelIndex &parent) const
{
    return m_hdrName.length();
}

Qt::DropActions QKxFtpTransferModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

#define CFG_VERSION     (11)
void QKxFtpTransferModel::load()
{
    QString path = m_pathTask + QString("/task%1.cfg").arg(m_name);
    QFile fi(path);
    int version;
    if(fi.open(QIODevice::ReadOnly)){
        QDataStream ds(&fi);
        emit beginResetModel();
        ds >> version;
        if(version == CFG_VERSION) {
            ds >> m_tasks;
            for(int i = 0; i < m_tasks.length(); i++) {
                TaskInfo &ti = m_tasks[i];
                if(ti.status == ETS_Finish) {
                    ti.percent = 100;
                }else{
                    if(ti.status == ETS_Transfer) {
                        ti.status = ETS_Pause;
                    }
                }
                ti.position = ti.fileSize * ti.percent / 100;
            }
        }
        emit endResetModel();
    }
}

void QKxFtpTransferModel::save()
{
    QString path = m_pathTask + QString("/task%1.cfg").arg(m_name);
    QFile fi(path);
    int version = CFG_VERSION;
    if(fi.open(QIODevice::WriteOnly)){
        QDataStream ds(&fi);
        ds << version << m_tasks;
    }
}

bool QKxFtpTransferModel::hasTask(const QString &localFile, const QString &remoteFile)
{
    for(QList<TaskInfo>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++) {
        TaskInfo& ti = *it;
        if(ti.localFile == localFile && ti.remoteFile == remoteFile) {
            return true;
        }
    }
    return false;
}

void QKxFtpTransferModel::startTransfer(int i, bool fromZero)
{
    if(i < 0 || i >= m_tasks.length()) {
        return;
    }
    // stop
    stopTransfer();

    // start.
    TaskInfo& ti = m_tasks[i];
    m_transferIndex = i;
    ti.status = ETS_Transfer;
    qDebug() << "startTransfer" << ti.isUpload;
    if(ti.isUpload) {
        if(m_upload == nullptr) {
            m_upload = new QKxFtpUpload(m_ftpRequest, this);
            QObject::connect(m_upload, SIGNAL(finished()), this, SLOT(onNextTransfer()));
            QObject::connect(m_upload, SIGNAL(error(int,QString)), this, SLOT(onTransferError(int,QString)));
            QObject::connect(m_upload, SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int,int)));
        }
        m_upload->stop();
        m_upload->start(ti.localFile, ti.remoteFile, fromZero);
    }else{
        if(m_download == nullptr) {
            m_download = new QKxFtpDownload(m_ftpRequest, this);
            QObject::connect(m_download, SIGNAL(finished()), this, SLOT(onNextTransfer()));
            QObject::connect(m_download, SIGNAL(error(int,QString)), this, SLOT(onTransferError(int,QString)));
            QObject::connect(m_download, SIGNAL(progress(int,int,int)), this, SLOT(onProgress(int,int,int)));
        }
        m_download->stop();
        m_download->start(ti.localFile, ti.remoteFile, fromZero);
    }
    m_saveLast = QDateTime::currentSecsSinceEpoch();
    updateItemView(m_transferIndex);
    save();
}

void QKxFtpTransferModel::startNextTransfer()
{
    if(m_transferIndex < 0 || m_transferIndex >= m_tasks.length()) {
        return;
    }
    int transferLast = m_transferIndex;
    TaskInfo& ti = m_tasks[m_transferIndex];
    if(ti.status == ETS_Transfer){
        m_transferIndex = -1;
        if(ti.percent == 100) {
            ti.status = ETS_Finish;
        }else{
            ti.status = ETS_Pause;
        }
    }else if(ti.status == ETS_Ready){
        ti.status = ETS_Pause;
    }
    int i = next(transferLast+1, ETS_Ready);
    if(i < 0) {
        i = first(ETS_Ready);
    }
    if(transferLast == i) {
        return;
    }
    startTransfer(i);
    save();
}

void QKxFtpTransferModel::stopTransfer()
{
    if(m_transferIndex < 0 || m_transferIndex >= m_tasks.length()) {
        return;
    }
    TaskInfo& ti = m_tasks[m_transferIndex];
    if(ti.status == ETS_Transfer) {
        ti.status = ETS_Pause;
    }
    m_transferIndex = -1;
    qDebug() << "stopTransfer" << ti.isUpload;
    if(m_upload) {
        m_upload->stop();
    }
    if(m_download){
        m_download->stop();
    }
    updateItemView(m_transferIndex);
}

void QKxFtpTransferModel::updateItemView(int i)
{
    QModelIndex idx = createIndex(i, 0, quintptr(0));
    emit taskUpdate(idx);
}

void QKxFtpTransferModel::onNextTransfer()
{
    updateItemView(m_transferIndex);
    startNextTransfer();
}

void QKxFtpTransferModel::onProgress(int percent, int position, int fsize)
{
    if(m_transferIndex < 0 || m_transferIndex >= m_tasks.length()) {
        return;
    }
    qDebug() << "onProgress" << percent;
    TaskInfo& ti = m_tasks[m_transferIndex];
    ti.percent = percent;
    ti.fileSize = fsize;
    ti.status = ETS_Transfer;
    ti.position = position;
    qint64 now = QDateTime::currentSecsSinceEpoch();
    if(now - m_saveLast > 10) {
        m_saveLast = now;
        save();
    }
    updateItemView(m_transferIndex);
}

void QKxFtpTransferModel::onTransferError(int err, const QString &msg)
{
    if(m_transferIndex < 0 || m_transferIndex >= m_tasks.length()) {
        return;
    }
    TaskInfo& ti = m_tasks[m_transferIndex];
    ti.status = ETS_Error;
    updateItemView(m_transferIndex);
    startNextTransfer();
}
