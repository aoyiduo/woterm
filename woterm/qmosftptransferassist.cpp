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

#include "qmosftptransferassist.h"
#include "qwosetting.h"
#include "qmosftpqueuemodel.h"
#include "qwossh.h"
#include "qkxmessagebox.h"
#include "qwoutils.h"

#include <QWidget>
#include <QTimer>
#include <QFileInfo>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QQuickItem>
#include <QQmlEngine>

#define MAX_LOG_SIZE    (1024 * 1024 * 5)

QMoSftpTransferAssist::QMoSftpTransferAssist(const QString &target, int gid, QQmlEngine *engine,  QWidget *parent)
    : QObject(parent)
    , m_target(target)
    , m_gid(gid)
    , m_queueMode(true)
    , m_abortExpected(EAbortNone)
    , m_engine(engine)
    , m_dlgWidget(parent)
{
    QString logPath = QWoSetting::sftpTaskLogPath();
    QString logFile = logPath + "/task.log";
    if(QFile::exists(logFile)) {
        QFileInfo fi(logFile);
        if(fi.size() > MAX_LOG_SIZE) {
            QFile::remove(logFile);
        }
    }
    m_logFile = logFile;
    m_taskModel = new QMoSftpQueueModel(this);
    QObject::connect(m_taskModel, SIGNAL(modelReset()), this, SLOT(onModelReset()), Qt::QueuedConnection);
    resetAll();
}

QMoSftpTransferAssist::~QMoSftpTransferAssist()
{
}

void QMoSftpTransferAssist::init(QObject *root)
{
    if(root == nullptr) {
        return;
    }
    m_btnStart = root->findChild<QQuickItem*>("btnStart");
    m_btnStop = root->findChild<QQuickItem*>("btnStop");
    m_btnAbort = root->findChild<QQuickItem*>("btnAbort");
    m_btnAbortAll = root->findChild<QQuickItem*>("btnAbortAll");
}

QMoSftpQueueModel *QMoSftpTransferAssist::taskModel()
{
    return m_taskModel;
}

void QMoSftpTransferAssist::onTransferStart()
{
    if(m_btnStart) {
        m_btnStart->setVisible(false);
    }
    if(m_btnStop) {
        m_btnStop->setVisible(true);
    }
    if(!m_taskDetail.isEmpty()) {
        int taskId = m_taskDetail.value("taskId").toInt();
        QString command = m_taskDetail.value("command").toString();
        QString local = m_taskDetail.value("local").toString();
        QString remote = m_taskDetail.value("remote").toString();
        bool append = m_taskDetail.value("append").toBool();
        if(command == "upload") {
            uploadFile(local, remote, append, taskId);
        }else if(command == "download") {
            downloadFile(remote, local, append, taskId);
        }else if(command == "listFile") {
            downloadDir(remote, local, taskId);
        }else if(command == "mkDir") {
            remoteMkDir(remote, taskId);
        }
    }
}

void QMoSftpTransferAssist::onTransferStop()
{
    release();
    m_abortExpected = EAbortAsStop;
    if(m_btnStart) {
        m_btnStart->setVisible(true);
    }
    if(m_btnStop) {
        m_btnStop->setVisible(false);
    }
}

void QMoSftpTransferAssist::onTransferAbort()
{
    if(m_btnStart && m_btnStart->isVisible()) {
        if(!runNexTask()) {
            emit hideArrived();
        }
        return;
    }
    if(m_sftp) {
        m_sftp->abort();
    }
    m_abortExpected = EAbortIt;
    if(m_taskModel->isEmpty()) {
        emit hideArrived();
    }
}

void QMoSftpTransferAssist::onTransferAbortAll()
{
    if(m_taskModel) {
        m_taskModel->removeAll();
    }

    if(m_sftp) {
        m_sftp->abort();
    }
    m_abortExpected = EAbortAll;
    m_taskModel->removeAll();
    emit hideArrived();
}

void QMoSftpTransferAssist::onTransferCommandStart(int type, const QVariantMap& userData)
{
    //qDebug() << "onTransferCommandStart" << type << userData;
}

void QMoSftpTransferAssist::onTransferCommandFinish(int type, const QVariantMap& userData)
{
    //qDebug() << "onTransferCommandFinish" << type << userData;
    int taskId = userData.value("taskId", -1).toInt();
    Q_ASSERT(taskId > 0);
    QString reason = userData.value("reason").toString();
    if(reason == "abort") {
        if(m_abortExpected == EAbortIt) {
            if(!runNexTask()) {
                emit hideArrived();
            }
        }else if(m_abortExpected == EAbortAll) {
            if(m_taskModel) {
                m_taskModel->removeAll();
            }
            emit hideArrived();
        }
        m_abortExpected = EAbortNone;
        return;
    }else if(reason == "error") {
        onTransferStop();
        QString err = userData.value("errorString").toString();
        if(!err.isEmpty()) {
            QKxMessageBox::warning(m_dlgWidget, tr("Error"), err, QMessageBox::Ok);
        }
        return;
    }
    if(type == 18) {
        if(!runNextSubTask(taskId)) {
            if(!runNexTask()) {
                emit hideArrived();
            }
        }
    }else if(m_taskRun.isDir) {
        if(!runNextSubTask(m_taskRun.taskId)) {
            if(!runNexTask()) {
                emit hideArrived();
            }
        }
    }else{
        if(!runNexTask()) {
            emit hideArrived();
        }
    }
}

void QMoSftpTransferAssist::onTransferProgress(int type, int v, const QVariantMap& userData)
{
}

void QMoSftpTransferAssist::onFinishArrived(int code)
{
    QWoSshFtp *ftp = qobject_cast<QWoSshFtp*>(sender());
    if(ftp != nullptr) {
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}

void QMoSftpTransferAssist::onTransferErrorArrived(const QString &msg, const QVariantMap &userData)
{
    if(userData.contains("taskId")) {
        int taskId = userData.value("taskId", -1).toInt();
        Q_ASSERT(taskId > 0);
        if(m_taskRun.taskId == taskId) {
#if 0
            if(!userData.contains("skipFile")) {
                m_taskRun.tryCount++;
                m_taskModel->append(m_taskRun);
                m_taskRun.clear();
            }
#endif
        }
    }
}

void QMoSftpTransferAssist::onTaskRemoveArrived(int tid)
{
    m_taskModel->removeTask(tid);
}

void QMoSftpTransferAssist::onModelReset()
{
    if(m_btnAbortAll) {
        m_btnAbortAll->setVisible(!(m_taskModel && m_taskModel->isEmpty()));
    }
}

void QMoSftpTransferAssist::onInputArrived(const QString &host, const QString &prompt, bool visble)
{
    // just only do transfer.
    m_sftp->stop();
}

void QMoSftpTransferAssist::onFileListArrived(const QVariantList &files, const QVariantMap& userData)
{
    m_taskFiles.append(files);
    m_taskRun.fileCount = m_taskFiles.length();
}

void QMoSftpTransferAssist::reconnect()
{
    if(m_sftp) {
        m_sftp->abort();
        QWoSshFactory::instance()->release(m_sftp);
    }
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SIGNAL(commandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SIGNAL(commandFinish(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onTransferCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onTransferCommandFinish(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onTransferErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(progress(int,int,QVariantMap)), this, SIGNAL(progress(int,int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(fileList(QVariantList,QVariantMap)), this, SLOT(onFileListArrived(QVariantList,QVariantMap)));

    m_sftp->setLogFile(m_logFile);
    m_sftp->start(m_target, m_gid);
}

void QMoSftpTransferAssist::runTask(const TaskInfo& ti)
{
    m_taskRun = ti;
    m_taskFiles.clear();
    m_elidedText = false;
    if(ti.isDir) {
        if(ti.isDown) {
            downloadDir(ti.remote, ti.local, ti.taskId);
        }else{
            uploadDir(ti.local, ti.remote, ti.taskId);
        }
    }else{
        if(ti.isDown) {
            downloadFile(ti.remote, ti.local, ti.isAppend, ti.taskId);
        }else{
            uploadFile(ti.local, ti.remote, ti.isAppend, ti.taskId);
        }
    }
}

bool QMoSftpTransferAssist::runNexTask()
{
    m_taskRun.clear();
    if(m_taskModel->isEmpty()) {
        return false;
    }
    const TaskInfo& ti = m_taskModel->takeFirst();
    runTask(ti);
    return true;
}

bool QMoSftpTransferAssist::runNextSubTask(int tid)
{
    if(m_taskRun.taskId != tid) {
        return false;
    }
    if(m_taskFiles.isEmpty()) {
        return false;
    }
    QVariant file = m_taskFiles.takeFirst();
    QVariantMap fileMap = file.toMap();
    QString filePath = fileMap.value("filePath").toString();
    bool isDir = fileMap.value("isDir", false).toBool();
    QString fileLocal = QDir::cleanPath(m_taskRun.local + "/" + filePath);
    QString fileRemote = QDir::cleanPath(m_taskRun.remote + "/" + filePath);
    if(m_taskRun.isDown){
        if(isDir) {
            // ignore it.
        }else{
            downloadFile(fileRemote, fileLocal, m_taskRun.isAppend, m_taskRun.taskId);
        }
    }else{
        if(isDir) {
            remoteMkDir(fileRemote, m_taskRun.taskId);
        }else{
            uploadFile(fileLocal, fileRemote, m_taskRun.isAppend, m_taskRun.taskId);
        }
    }
    return true;
}

void QMoSftpTransferAssist::remoteMkDir(const QString &_remote, int tid)
{
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString remote = QDir::cleanPath(_remote);
    QVariantMap dm;
    dm.insert("taskId", tid);
    dm.insert("remote", remote);
    dm.insert("command", "mkDir");
    m_taskDetail = dm;
    m_sftp->mkDir(remote, 0x1FF, dm);
}

bool QMoSftpTransferAssist::uploadDir(const QString &_local, const QString &_remote, int tid)
{
    if(m_sftp == nullptr) {
        reconnect();
    }
    m_taskFiles.clear();
    m_taskLocalPaths.clear();
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    listFile(local);
    return true;
}

bool QMoSftpTransferAssist::uploadFile(const QString &_local, const QString &_remote, bool append, int tid)
{
    append = m_queueMode && append;
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    QVariantMap dm;
    dm.insert("taskId", tid);
    dm.insert("remote", remote);
    dm.insert("local", local);
    dm.insert("append", append);
    dm.insert("command", "upload");
    m_taskDetail = dm;
    m_sftp->upload(local, remote, append ? QWoSshFtp::TP_Append : QWoSshFtp::TP_Override, dm);
    return true;
}

bool QMoSftpTransferAssist::downloadDir(const QString &_remote, const QString &_local, int tid)
{
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    QVariantMap dm;
    dm.insert("taskId", tid);
    dm.insert("basePath", remote);
    dm.insert("command", "listFile");
    m_taskDetail = dm;
    m_sftp->listFile(remote, dm);
    return true;
}

bool QMoSftpTransferAssist::downloadFile(const QString &_remote, const QString &_local, bool append, int tid)
{
    append = m_queueMode && append;
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    QVariantMap dm;
    dm.insert("taskId", tid);
    dm.insert("remote", remote);
    dm.insert("local", local);
    dm.insert("append", append);
    dm.insert("command", "download");
    m_taskDetail = dm;
    m_sftp->download(remote, local, append ? QWoSshFtp::TP_Append : QWoSshFtp::TP_Override, dm);
    return true;
}

int QMoSftpTransferAssist::localTaskId()
{
    static int tid = 1;
    tid++;
    return tid;
}

void QMoSftpTransferAssist::resetAll()
{
    m_abortExpected = EAbortNone;
    m_taskFiles.clear();
    m_taskLocalPaths.clear();
    m_taskRun.clear();
    m_taskDetail.clear();
    if(m_taskModel) {
        m_taskModel->removeAll();
    }
    release();
    if(m_btnStart) {
        m_btnStart->setVisible(false);
    }
    if(m_btnStop) {
        m_btnStop->setVisible(false);
    }
}

void QMoSftpTransferAssist::outputFileList()
{
    for(int i = 0; i < m_taskFiles.length(); i++) {
        QVariantMap file = m_taskFiles.at(i).toMap();
        QString name = file.value("filePath").toString();
        qDebug() << QDir::cleanPath(name);
    }
}

void QMoSftpTransferAssist::listFile(const QString &path)
{
    {
        QVariantMap dm;
        QString filePath = path;
        filePath = filePath.replace(0, m_taskRun.local.length(), "./");
        dm.insert("isDir", true);
        dm.insert("filePath", filePath);
        m_taskFiles.append(dm);
    }
    QDir d(path);
    QFileInfoList lsfi = d.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        const QFileInfo& fi = *it;
        if(fi.isDir()) {
            m_taskLocalPaths.append(fi.absoluteFilePath());
        }else{
            QString filePath = fi.absoluteFilePath();
            filePath = filePath.replace(0, m_taskRun.local.length(), "./");
            QVariantMap dm;
            dm.insert("isDir", false);
            dm.insert("fileSize", fi.size());
            dm.insert("filePath", filePath);
            m_taskFiles.append(dm);
        }
    }
    m_taskRun.fileCount = m_taskFiles.length();
    if(m_taskLocalPaths.isEmpty()) {
        if(!runNextSubTask(m_taskRun.taskId)) {
            if(!runNexTask()) {
               emit hideArrived();
            }
        }
    }else{
        QString subPath = m_taskLocalPaths.takeFirst();
        QMetaObject::invokeMethod(this, "listFile", Qt::QueuedConnection, Q_ARG(QString, subPath));
    }
}

void QMoSftpTransferAssist::setTaskLabel(const QString &_local, const QString &_remote)
{
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    emit labelArrived(local, remote);
}

bool QMoSftpTransferAssist::addTask(const QString &pathLocal, const QString &pathRemote, bool isDir, bool isDown, bool isAppend)
{
    if(m_queueMode) {
        TaskInfo ti;
        ti.isAppend = isAppend;
        ti.isDir = isDir;
        ti.isDown = isDown;
        ti.local = pathLocal;
        ti.remote = pathRemote;
        ti.state = ETS_Ready;
        ti.taskId = localTaskId();
        if(m_taskRun.isValid()) {
            if(!m_taskModel->append(ti)) {
                return false;
            }
        }else{
            runTask(ti);
        }
        return true;
    }else if(m_taskRun.isValid()){
        return false;
    }else if(!isDir) {
        TaskInfo ti;
        ti.isAppend = isAppend;
        ti.isDir = isDir;
        ti.isDown = isDown;
        ti.local = pathLocal;
        ti.remote = pathRemote;
        ti.state = ETS_Ready;
        ti.taskId = localTaskId();
        runTask(ti);
        return true;
    }
    return false;
}

void QMoSftpTransferAssist::release()
{
    if(m_sftp) {
        m_sftp->abort();
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}
