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

#ifndef QKXFTPTRANSFERMODEL_H
#define QKXFTPTRANSFERMODEL_H

#include "qkxftp_share.h"

#include <QAbstractListModel>
#include <QFont>
#include <QIcon>
#include <QDataStream>
#include <QPointer>
#include <QList>
#include <QPair>
#include <QIcon>
#include <QStyledItemDelegate>



class QKxFtpRequest;
class QKxFtpDownload;
class QKxFtpUpload;
class KXFTP_EXPORT QKxFtpTransferModel : public QAbstractListModel
{
    Q_OBJECT    

public:
    enum ETaskStatus {
        ETS_Ready = 0, // transfer queue.
        ETS_Transfer = 1, // transfering.
        ETS_Pause = 2,
        ETS_Finish = 3,
        ETS_Error = 4
    };
    Q_ENUM(ETaskStatus)
    struct TaskInfo {
        QString name;
        bool isUpload;
        ETaskStatus status;
        QString localFile;
        QString remoteFile;
        int percent;
        int position;
        int fileSize;
        qint64 timeLast;
        TaskInfo() {
            status = ETS_Ready;
            isUpload = false;
            percent = timeLast = position = fileSize = 0;
        }
    };

public:
    explicit QKxFtpTransferModel(const QString& path, const QString& name, QObject *parent = nullptr);
    ~QKxFtpTransferModel();
    void setFtpRequest(QKxFtpRequest *ftp);
    void addTask(const QList<QString> &files, const QString& localPath, const QString& remotePath, bool isUpload);
    void start(const QModelIndex& idx, bool fromZero = false);
    void restart(const QModelIndex& idx);
    void startAll();
    void stop(const QModelIndex& idx);
    void stopAll();
    void remove(const QModelIndex& idx);
    void removeAll();
    int taskCount();
    int startCount();
    int stopCount();
    int first(ETaskStatus status);
    int next(int idx, ETaskStatus status);
    ETaskStatus taskStatus(const QModelIndex& idx);
signals:
    void taskUpdate(const QModelIndex& idx);
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::DropActions supportedDropActions() const override;
private:
    void test();
    Q_INVOKABLE void load();
    void save();
    bool hasTask(const QString& localFile, const QString& remoteFile);
    void startTransfer(int i, bool fromZero = false);
    void startNextTransfer();
    void stopTransfer();
    void updateItemView(int i);
private slots:
    void onNextTransfer();
    void onProgress(int percent, int position, int fsize);
    void onTransferError(int err,const QString& msg);
private:
    QIcon m_iconUpload, m_iconDown, m_iconReady, m_iconPause, m_iconTransfer, m_iconFinish;
    QString m_name;
    int m_transferIndex;
    qint64 m_saveLast;
    QList<TaskInfo> m_tasks;
    QString m_pathTask;
    QPointer<QKxFtpRequest> m_ftpRequest;
    QStringList m_hdrName;
    QPointer<QKxFtpDownload> m_download;
    QPointer<QKxFtpUpload> m_upload;
};

#endif // QKXFTPTRANSFERMODEL_H
