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

#ifndef QMOSFTPTRANSFERASSIST_H
#define QMOSFTPTRANSFERASSIST_H

#include "qwoglobal.h"

#include <QObject>
#include <QPointer>

class QWidget;
class QWoSshFtp;
class QMoSftpQueueModel;
class QQuickItem;
class QQmlEngine;

class QMoSftpTransferAssist : public QObject
{
    Q_OBJECT
    enum EAbortExpected {
         EAbortNone = 0,
         EAbortIt = 1,
         EAbortAll = 2,
         EAbortAsStop = 3
    };

public:
    explicit QMoSftpTransferAssist(const QString &target, int gid, QQmlEngine *engine, QWidget *parent = 0);
    ~QMoSftpTransferAssist();
    Q_INVOKABLE void init(QObject *root);
    Q_INVOKABLE QMoSftpQueueModel *taskModel();
    Q_INVOKABLE bool addTask(const QString& pathLocal, const QString& pathRemote, bool isDir, bool isDown, bool isAppend);
    Q_INVOKABLE void release();
    Q_INVOKABLE void resetAll();
signals:
    void commandStart(int type, const QVariantMap& userData);
    void commandFinish(int type, const QVariantMap& userData);
    void hideArrived();
    void labelArrived(const QString& local, const QString& remote);
    void progress(int type, int v, const QVariantMap& user);
public slots:
    void onTransferStart();
    void onTransferStop();
    void onTransferAbort();
    void onTransferAbortAll();
    void onTransferCommandStart(int type, const QVariantMap& userData);
    void onTransferCommandFinish(int type, const QVariantMap& userData);
    void onTransferProgress(int type, int v, const QVariantMap& userData);
    void onFinishArrived(int code);
    void onTransferErrorArrived(const QString& msg, const QVariantMap& userData);

    void onTaskRemoveArrived(int tid);
    void onModelReset();

    void onInputArrived(const QString& host, const QString& prompt, bool visble);
    void onFileListArrived(const QVariantList& files, const QVariantMap& userData);

private:
    void reconnect();
    void runTask(const TaskInfo& ti);
    bool runNexTask();
    bool runNextSubTask(int tid);
    void remoteMkDir(const QString& remote, int tid);
    bool uploadDir(const QString &local, const QString &remote, int tid);
    bool uploadFile(const QString &local, const QString &remote, bool append, int tid);
    bool downloadDir(const QString &remote, const QString &local, int tid);
    bool downloadFile(const QString &remote, const QString &local, bool append, int tid);
    QString logFilePath();
    int localTaskId();
    void outputFileList();
    Q_INVOKABLE void listFile(const QString& path);
    void setTaskLabel(const QString& local, const QString& remote);
private:
    QString m_target;
    int m_gid;
    bool m_queueMode;

    bool m_elidedText;
    EAbortExpected m_abortExpected;

    QPointer<QWoSshFtp> m_sftp;
    QList<QVariant> m_taskFiles;
    QStringList m_taskLocalPaths;
    QString m_logFile;
    TaskInfo m_taskRun;
    QVariantMap m_taskDetail;
    QPointer<QMoSftpQueueModel> m_taskModel;
    QPointer<QQuickItem> m_btnStart, m_btnStop, m_btnAbort, m_btnAbortAll;
    QPointer<QQmlEngine> m_engine;
    QPointer<QWidget> m_dlgWidget;
};

#endif // QMOSFTPTRANSFERASSIST_H
