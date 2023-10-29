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

#include "qwosftptransferwidget.h"
#include "ui_qwosftptransferwidget.h"
#include "qwosetting.h"
#include "qwosftpqueuemodel.h"
#include "qwohosttreemodel.h"
#include "qwossh.h"
#include "qwotreeview.h"
#include "qkxmessagebox.h"
#include "qwoutils.h"
#include "qkxlabelassist.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QFileInfo>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QHeaderView>
#include <QToolButton>
#include <QMessageBox>
#include <QScrollBar>

#define MAX_LOG_SIZE    (1024 * 1024 * 5)
#define BUTTON_WIDTH    (60)
#define BUTTON_HEIGHT   (30)

QWoSftpItemDelegate::QWoSftpItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_parent(parent)
{
    m_uploadIcon = QPixmap("../private/skins/black/upload.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation);
    m_downIcon = QPixmap("../private/skins/black/download.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation);
    m_dirIcon = QPixmap("../private/skins/black/dirs.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation);
    m_fileIcon = QPixmap("../private/skins/black/file.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation);
}

void QWoSftpItemDelegate::onRemoveArrived()
{
    QWidget *btnRemove = qobject_cast<QWidget*>(sender());
    const TaskInfo& ti = btnRemove->property("itemIndex").value<TaskInfo>();
    if(ti.isValid()) {
        emit removeArrived(ti.taskId);
    }
}

void QWoSftpItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const
{
    if (idx.column() == 0) {
        const TaskInfo& ti = idx.data(ROLE_TASKINFO).value<TaskInfo>();
        painter->save();
        if (option.state & QStyle::State_Selected){
            painter->fillRect(option.rect, option.palette.highlight());
        }
        QRect rt = option.rect;
        rt.adjust(2,2,-2,-2);
        int xoffset = rt.left();
        if(ti.isDown) {
            QSize sz = m_downIcon.size();
            int yoffset = (rt.height() - sz.height()) / 2;
            QRect subRt = QRect(xoffset, rt.top() + yoffset, sz.width(), sz.height());
            painter->drawPixmap(subRt, m_downIcon, QRectF(0, 0, sz.width(), sz.height()));
            xoffset += subRt.width();
        }else{
            QSize sz = m_uploadIcon.size();
            int yoffset = (rt.height() - sz.height()) / 2;
            QRect subRt = QRect(xoffset, rt.top() + yoffset, sz.width(), sz.height());
            painter->drawPixmap(subRt, m_uploadIcon, QRectF(0, 0, sz.width(), sz.height()));
            xoffset += subRt.width();
        }
        if(ti.isDir) {
            QSize sz = m_dirIcon.size();
            int yoffset = (rt.height() - sz.height()) / 2;
            QRect subRt = QRect(xoffset, rt.top() + yoffset, sz.width(), sz.height());
            painter->drawPixmap(subRt, m_dirIcon, QRectF(0, 0, sz.width(), sz.height()));
        }else{
            QSize sz = m_fileIcon.size();
            int yoffset = (rt.height() - sz.height()) / 2;
            QRect subRt = QRect(xoffset, rt.top() + yoffset, sz.width(), sz.height());
            painter->drawPixmap(subRt, m_fileIcon, QRectF(0, 0, sz.width(), sz.height()));
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, idx);
    }
}

QSize QWoSftpItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *QWoSftpItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *container = m_parent->property("container").value<QWidget*>();
    if(container == nullptr) {
        QIcon remove = QIcon(QPixmap("../private/skins/black/close.png").scaled(24, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
        QPushButton *btnRemove = new QPushButton(parent);
        btnRemove->setObjectName("transferRemove");
        btnRemove->setIcon(remove);
        btnRemove->setText(tr("Delete"));
        QObject::connect(btnRemove, SIGNAL(clicked()), this, SLOT(onRemoveArrived()));
        m_parent->setProperty("container", QVariant::fromValue<QWidget*>(btnRemove));
        m_parent->setProperty("btnRemove", QVariant::fromValue<QWidget*>(btnRemove));
        return btnRemove;
    }
    return container;
}

void QWoSftpItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{

}

void QWoSftpItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

}

void QWoSftpItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

}

void QWoSftpItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &idx) const
{
    if(!idx.isValid() || idx.column() == 0) {
        return;
    }
    QRect rt = option.rect;
    QRect btnRt(rt.left() + BUTTON_HEIGHT, rt.top() + (rt.height() - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT);
    qDebug() << "updateEditorGeometry" << btnRt << rt;
    editor->setGeometry(btnRt);
    editor->setProperty("itemIndex", idx.data(ROLE_TASKINFO));
}


QWoSftpTransferWidget::QWoSftpTransferWidget(const QString &target, int gid, bool queueMode, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoSftpTransferWidget)
    , m_target(target)
    , m_gid(gid)
    , m_queueMode(queueMode)
    , m_abortExpected(EAbortNone)
{
    ui->setupUi(this);
    if(queueMode) {
        QObject::connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(onTransferStart()));
        QObject::connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(onTransferStop()));
        QObject::connect(ui->btnAbortAll, SIGNAL(clicked()), this, SLOT(onTransferAbortAll()));
    }
    QObject::connect(ui->btnAbort, SIGNAL(clicked()), this, SLOT(onTransferAbort()));
    ui->fileProgress->setRange(0, 100);
    ui->fileProgress->setValue(0);
    ui->dirProgress->setRange(0, 100);
    ui->dirProgress->setValue(0);
    ui->dirProgress->setVisible(false);
    ui->taskArea->setVisible(true);
    QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onLocalFilePathBrowser()));
    QObject::connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(onRemoteFilePathCopy()));

    QString logPath = QWoSetting::sftpTaskLogPath();
    QString logFile = logPath + "/task.log";
    if(QFile::exists(logFile)) {
        QFileInfo fi(logFile);
        if(fi.size() > MAX_LOG_SIZE) {
            QFile::remove(logFile);
        }
    }
    m_logFile = logFile;
    m_delegate = new QWoSftpItemDelegate(this);
    m_taskModel = new QWoSftpQueueModel(this);
    m_tree = new QWoTreeView(this);
    ui->taskLayout->addWidget(m_tree);
    m_tree->setModel(m_taskModel);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
    m_tree->setIconSize(QSize(20, 20));
    m_tree->setIndentation(0);
    m_tree->setItemDelegate(m_delegate);
    m_tree->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QObject::connect(m_delegate, SIGNAL(removeArrived(int)), this, SLOT(onTaskRemoveArrived(int)));
    QObject::connect(m_taskModel, SIGNAL(modelReset()), this, SLOT(onModelReset()), Qt::QueuedConnection);
    QObject::connect(m_tree, SIGNAL(pressed(QModelIndex)), SLOT(onTreeItemPressed(QModelIndex)));

    m_progressLabel = new QKxLabelAssist(ui->fileProgress);

    ui->fileLocal->installEventFilter(this);
    ui->fileRemote->installEventFilter(this);
    setTaskLabel(ui->fileLocal->text(), ui->fileRemote->text());
    resetAll();
}

QWoSftpTransferWidget::~QWoSftpTransferWidget()
{
    delete ui;
}

bool QWoSftpTransferWidget::isQueueMode()
{
    return m_queueMode;
}

void QWoSftpTransferWidget::onAdjustLayout()
{
    adjustSize();
}

void QWoSftpTransferWidget::onLocalFilePathBrowser()
{
    QString url = ui->fileLocal->property("fileLocal").toString();
    if(url.isEmpty()) {
        return;
    }
    QFileInfo fi(url);
    if(!fi.isDir()) {
        QString path = fi.absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(url));
}

void QWoSftpTransferWidget::onRemoteFilePathCopy()
{
    QString fileRemote = ui->fileRemote->property("fileRemote").toString();
    if(fileRemote.isEmpty()) {
        return;
    }
    QGuiApplication::clipboard()->setText(fileRemote);
}

void QWoSftpTransferWidget::onModelReset()
{
    for(int i = 0; i < 2; i++) {
        m_tree->resizeColumnToContents(i);
    }
    if(m_taskModel->isEmpty()) {
        ui->taskArea->hide();
        ui->btnAbortAll->hide();
    }else{
        ui->btnAbortAll->show();
    }
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
}

void QWoSftpTransferWidget::onTransferStart()
{
    ui->btnStop->setVisible(m_queueMode);
    ui->btnStart->hide();
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

void QWoSftpTransferWidget::onTransferStop()
{
    release();
    m_abortExpected = EAbortAsStop;
    ui->btnStop->hide();
    ui->btnStart->setVisible(m_queueMode);
}

void QWoSftpTransferWidget::onTransferAbort()
{
    if(ui->btnStart->isVisible()) {
        if(!runNexTask()) {
            hide();
        }
        return;
    }
    if(m_sftp) {
        m_sftp->abort();
    }
    m_abortExpected = EAbortIt;
    setVisible(!m_taskModel->isEmpty());
}

void QWoSftpTransferWidget::onTransferAbortAll()
{
    if(m_taskModel) {
        m_taskModel->removeAll();
    }

    if(m_sftp) {
        m_sftp->abort();
    }
    m_abortExpected = EAbortAll;
    m_taskModel->removeAll();
    setVisible(false);
}

void QWoSftpTransferWidget::onTransferCommandStart(int type, const QVariantMap& userData)
{
    raise();
}

void QWoSftpTransferWidget::onTransferCommandFinish(int type, const QVariantMap& userData)
{
    int taskId = userData.value("taskId", -1).toInt();
    Q_ASSERT(taskId > 0);
    QString reason = userData.value("reason").toString();
    if(reason == "abort") {
        if(m_abortExpected == EAbortIt) {
            if(!runNexTask()) {
                hide();
            }
        }else if(m_abortExpected == EAbortAll) {
            if(m_taskModel) {
                m_taskModel->removeAll();
            }
            hide();
        }
        m_abortExpected = EAbortNone;
        return;
    }else if(reason == "error") {
        onTransferStop();
        QString err = userData.value("errorString").toString();
        if(!err.isEmpty()) {
            QKxMessageBox::warning(this, tr("Error"), err, QMessageBox::Ok);
        }
        return;
    }
    if(type == 18) {
        if(!runNextSubTask(taskId)) {
            if(!runNexTask()) {
                hide();
            }
        }
    }else if(m_taskRun.isDir) {
        if(!runNextSubTask(m_taskRun.taskId)) {
            if(!runNexTask()) {
                hide();
            }
        }
    }else{
        if(!runNexTask()) {
            hide();
        }
    }
}

void QWoSftpTransferWidget::onTransferProgress(int type, int v, const QVariantMap& userData)
{
    ui->fileProgress->setValue(v);
    if(userData.contains("speed")) {
        m_progressLabel->setVisible(true);
        double speed = userData.value("speed").toDouble();
        QString out;
        if(speed > 1024 * 1024) {
            double fval = speed / 1024 / 1024;
            out = QString::number(fval, 'f', 1);
            out.append("MB/s");
        }else if(speed > 1024){
            double fval = speed / 1024;
            out = QString::number(fval, 'f', 1);
            out.append("KB/s");
        }else{
            out = QString::number(speed, 'f', 0);
            out.append("B/s");
        }
        m_progressLabel->setText(out);
    }else{
        m_progressLabel->setVisible(false);
    }
}

void QWoSftpTransferWidget::onFinishArrived(int code)
{
    QWoSshFtp *ftp = qobject_cast<QWoSshFtp*>(sender());
    if(ftp != nullptr) {
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}

void QWoSftpTransferWidget::onTransferErrorArrived(const QString &msg, const QVariantMap &userData)
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

void QWoSftpTransferWidget::onTaskRemoveArrived(int tid)
{
    m_taskModel->removeTask(tid);
}

void QWoSftpTransferWidget::onInputArrived(const QString &host, const QString &prompt, bool visble)
{
    // just only do transfer.
    m_sftp->stop();
}

void QWoSftpTransferWidget::onFileListArrived(const QVariantList &files, const QVariantMap& userData)
{
    m_taskFiles.append(files);
    m_taskRun.fileCount = m_taskFiles.length();
    ui->fileCount->setText(QString::number(m_taskRun.fileCount));
    ui->dirProgress->setValue(0);
    ui->dirProgress->setMinimum(0);
    ui->dirProgress->setMaximum(m_taskRun.fileCount);
}

void QWoSftpTransferWidget::onTreeItemPressed(const QModelIndex &index)
{
    //QScrollBar *bar = m_tree->horizontalScrollBar();
    //if(bar) {
    //    int v = bar->maximum();
    //    bar->setValue(v);
    //}
}

QWoSftpTask *QWoSftpTransferWidget::get()
{
    if(m_task == nullptr) {
        m_task = new QWoSftpTask(this);
    }
    return m_task;
}

void QWoSftpTransferWidget::reconnect()
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
    QObject::connect(m_sftp, SIGNAL(progress(int,int,QVariantMap)), this, SLOT(onTransferProgress(int,int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(fileList(QVariantList,QVariantMap)), this, SLOT(onFileListArrived(QVariantList,QVariantMap)));

    m_sftp->setLogFile(m_logFile);
    m_sftp->start(m_target, m_gid);
}

void QWoSftpTransferWidget::runTask(const TaskInfo& ti)
{
    m_taskRun = ti;
    m_taskFiles.clear();
    ui->btnStart->hide();
    ui->btnStop->setVisible(m_queueMode);
    QWoUtils::setLayoutVisible(ui->dirLayout, ti.isDir);
    if(ti.isDir) {
        ui->fileCount->setText(QString::number(m_taskFiles.length()));
    }
    if(isPrivateSkinPath(ti.local)) {
        QKxMessageBox::warning(this, tr("Permission error"), tr("It is not allowed to directly download or upload files related to the skin directory, which will cause confusion in skin loading."));
        return;
    }
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

bool QWoSftpTransferWidget::runNexTask()
{
    m_taskRun.clear();
    if(m_taskModel->isEmpty()) {
        resetAll();
        return false;
    }
    const TaskInfo& ti = m_taskModel->takeFirst();
    runTask(ti);
    return true;
}

bool QWoSftpTransferWidget::runNextSubTask(int tid)
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
    QWoUtils::setLayoutVisible(ui->dirLayout, m_taskRun.isDir);
    ui->fileCount->setText(QString::number(m_taskFiles.length()));
    ui->dirProgress->setValue(m_taskRun.fileCount - m_taskFiles.length());
    return true;
}

void QWoSftpTransferWidget::remoteMkDir(const QString &_remote, int tid)
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

bool QWoSftpTransferWidget::uploadDir(const QString &_local, const QString &_remote, int tid)
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

bool QWoSftpTransferWidget::uploadFile(const QString &_local, const QString &_remote, bool append, int tid)
{
    append = m_queueMode && append;
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    ui->btnType->setIcon(QIcon("../private/skins/black/upload.png"));
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

bool QWoSftpTransferWidget::downloadDir(const QString &_remote, const QString &_local, int tid)
{
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    ui->btnType->setIcon(QIcon("../private/skins/black/download.png"));
    QVariantMap dm;
    dm.insert("taskId", tid);
    dm.insert("basePath", remote);
    dm.insert("command", "listFile");
    m_taskDetail = dm;
    m_sftp->listFile(remote, dm);
    return true;
}

bool QWoSftpTransferWidget::downloadFile(const QString &_remote, const QString &_local, bool append, int tid)
{
    append = m_queueMode && append;
    if(m_sftp == nullptr) {
        reconnect();
    }
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);
    setTaskLabel(local, remote);
    ui->btnType->setIcon(QIcon("../private/skins/black/download.png"));
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

int QWoSftpTransferWidget::localTaskId()
{
    static int tid = 1;
    tid++;
    return tid;
}

void QWoSftpTransferWidget::resetAll()
{
    m_abortExpected = EAbortNone;
    m_taskFiles.clear();
    m_taskRun.clear();
    m_taskDetail.clear();
    if(m_taskModel){
        m_taskModel->removeAll();
    }
    ui->btnAbortAll->setVisible(false);
    ui->btnStart->setVisible(false);
    ui->btnStop->setVisible(m_queueMode);
    ui->taskArea->setVisible(false);
}

void QWoSftpTransferWidget::outputFileList()
{
    for(int i = 0; i < m_taskFiles.length(); i++) {
        QVariantMap file = m_taskFiles.at(i).toMap();
        QString name = file.value("filePath").toString();
        qDebug() << QDir::cleanPath(name);
    }
}

void QWoSftpTransferWidget::listFile(const QString &path)
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
    ui->fileCount->setText(QString::number(m_taskRun.fileCount));
    ui->dirProgress->setValue(0);
    ui->dirProgress->setMinimum(0);
    ui->dirProgress->setMaximum(m_taskRun.fileCount);
    if(m_taskLocalPaths.isEmpty()) {
        if(!runNextSubTask(m_taskRun.taskId)) {
            if(!runNexTask()) {
                hide();
            }
        }
    }else{
        QString subPath = m_taskLocalPaths.takeFirst();
        QMetaObject::invokeMethod(this, "listFile", Qt::QueuedConnection, Q_ARG(QString, subPath));
    }
}

void QWoSftpTransferWidget::setTaskLabel(const QString &_local, const QString &_remote)
{
    QString local = QDir::cleanPath(_local);
    QString remote = QDir::cleanPath(_remote);


    ui->fileLocal->setProperty("fileLocal", local);
    ui->fileRemote->setProperty("fileRemote", remote);

    QFontMetrics fm1(ui->fileLocal->font());
    QFontMetrics fm2(ui->fileRemote->font());
    local = fm1.elidedText(local, Qt::ElideLeft, ui->fileLocal->width());
    remote = fm2.elidedText(remote, Qt::ElideLeft, ui->fileRemote->width());

    ui->fileLocal->setText(local);
    ui->fileRemote->setText(remote);
}

bool QWoSftpTransferWidget::isPrivateSkinPath(const QString &file)
{
    static QString skinRelativePrefix = "../private/skins";
    static QString skinsPath;
    if(skinsPath.isEmpty()) {
        skinsPath = QDir::cleanPath(QWoSetting::privatePath() + "/skins");
        skinsPath = skinsPath.toLower();
    }
    if(file.startsWith(skinRelativePrefix)) {
        return true;
    }
    QString fileLower = file.toLower();
    if(fileLower.contains(skinsPath)) {
        return true;
    }
    return false;
}

bool QWoSftpTransferWidget::canAddTask()
{
    if(m_queueMode) {
        return true;
    }
    return !m_taskRun.isValid();
}

bool QWoSftpTransferWidget::addTask(const QString &pathLocal, const QString &pathRemote, bool isDir, bool isDown, bool isAppend)
{
    if(m_queueMode) {
        TaskInfo ti;
        //QWoSftpTask *task = get();
        //int tid = task->addTask(m_target, pathLocal, pathRemote, isDir, isDown);
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
            ui->taskArea->setVisible(true);
        }else{
            ui->taskArea->setVisible(false);
            runTask(ti);
        }
        setVisible(true);
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
        setVisible(true);        
        ui->taskArea->setVisible(false);
        return true;
    }
    return false;
}

void QWoSftpTransferWidget::release()
{
    if(m_sftp) {
        m_sftp->abort();
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}

void QWoSftpTransferWidget::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);
    if(m_queueMode) {
    }else{
        ui->fileProgress->setValue(0);
        ui->dirProgress->setValue(0);
    }
    QTimer::singleShot(0, this, SLOT(onAdjustLayout()));
    raise();
}

void QWoSftpTransferWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void QWoSftpTransferWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void QWoSftpTransferWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void QWoSftpTransferWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

bool QWoSftpTransferWidget::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type t = ev->type();
    if(obj == ui->fileLocal) {
        if(t == QEvent::Resize) {
            QString local = obj->property("fileLocal").toString();
            QFontMetrics fm(ui->fileLocal->font());
            local = fm.elidedText(local, Qt::ElideLeft, ui->fileLocal->width());
            ui->fileLocal->setText(local);
        }
    }else if(obj == ui->fileRemote) {
        if(t == QEvent::Resize) {
            QString remote = obj->property("fileRemote").toString();
            QFontMetrics fm(ui->fileRemote->font());
            remote = fm.elidedText(remote, Qt::ElideLeft, ui->fileRemote->width());
            ui->fileRemote->setText(remote);
        }
    }
    return QWidget::eventFilter(obj, ev);
}
