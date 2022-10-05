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

#include "qkxftptransferwidget.h"
#include "qkxftptransfermodel.h"
#include "qkxftprequest.h"
#include "qkxftplocalmodel.h"
#include "qkxftpremotemodel.h"

#include <QDebug>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QHeaderView>
#include <QListView>
#include <QFileSystemModel>
#include <QAction>
#include <QInputDialog>
#include <QFileIconProvider>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QResizeEvent>
#include <QClipboard>
#include <QGuiApplication>


QKxProgress::QKxProgress(QWidget *parent)
    : QStyledItemDelegate(parent)
{

}

#define PROGRESS_BARHEIGHT (10)
void QKxProgress::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const
{
    if (idx.column() == 2) {
        int percent = idx.data().toInt();
        percent = qBound(0, percent, 100);
        //qDebug() << "percent" << percent;
        painter->save();
        if (option.state & QStyle::State_Selected){
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(Qt::white);
        }else{
            painter->setPen(Qt::black);
        }
        int yd = (option.rect.height() - PROGRESS_BARHEIGHT) / 2;
        int wd = (option.rect.width() - 10) * percent / 100;
        QRect rt = option.rect.adjusted(5, yd, -5, -yd);
        if(percent > 0) {
            QRect frt = rt;
            frt.setWidth(wd);
            painter->fillRect(frt, Qt::green);
        }
        painter->drawRect(rt);
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, idx);
    }
}

QKxFtpTransferWidget::QKxFtpTransferWidget(const QString& path, const QString& name, QWidget *parent)
    : QWidget(parent)
    , m_pathTask(path)
    , m_name(name)
{
    setWindowTitle(tr("File Transfer"));
    setContentsMargins(0, 0, 0, 0);
    setObjectName("fileTransfer");
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    m_root = new QSplitter(this);
    m_root->setHandleWidth(3);
    m_root->setOrientation(Qt::Vertical);
    m_root->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_root);
    setLayout(layout);

    QWidget *localArea = new QWidget(m_root);
    localArea->setContentsMargins(0, 0, 0, 0);
    QWidget *remoteArea= new QWidget(m_root);
    remoteArea->setContentsMargins(0, 0, 0, 0);   
    QSplitter *top = new QSplitter(m_root);
    top->setHandleWidth(3);
    top->setOrientation(Qt::Horizontal);
    top->setContentsMargins(0, 0, 0, 0);
    top->addWidget(localArea);
    top->addWidget(remoteArea);
    m_root->addWidget(top);
    QWidget *transferArea = new QWidget(m_root);
    transferArea->setContentsMargins(0, 0, 0, 0);
    m_root->addWidget(transferArea);
    QList<int> ls;
    ls.append(3);
    ls.append(1);
    m_root->setSizes(ls);
    {
        QVBoxLayout *layout = new QVBoxLayout(localArea);
        layout->setSpacing(3);
        localArea->setLayout(layout);
        layout->addWidget(new QLabel(tr("Local folder"), localArea));
        QHBoxLayout *tool = new QHBoxLayout(localArea);
        layout->addLayout(tool);

        QPushButton *btnHome = new QPushButton(localArea);
        QObject::connect(btnHome, SIGNAL(clicked(bool)), this, SLOT(onLocalHomeButtonClicked()));
        btnHome->setObjectName("ftp");
        btnHome->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/home.png").scaled(24, 24)));
        tool->addWidget(btnHome);
        QPushButton *btnReturn = new QPushButton(localArea);
        QObject::connect(btnReturn, SIGNAL(clicked(bool)), this, SLOT(onLocalBackButtonClicked()));
        btnReturn->setObjectName("ftp");
        btnReturn->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/back.png").scaled(24, 24)));
        tool->addWidget(btnReturn);
        QPushButton *btnEnter = new QPushButton(localArea);
        QObject::connect(btnEnter, SIGNAL(clicked(bool)), this, SLOT(onLocalForwardButtonClicked()));
        btnEnter->setObjectName("ftp");
        btnEnter->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/forward.png").scaled(24, 24)));
        tool->addWidget(btnEnter);

        QPushButton *btnRefresh = new QPushButton(localArea);
        QObject::connect(btnRefresh, SIGNAL(clicked(bool)), this, SLOT(onLocalRefreshButtonClicked()));
        btnRefresh->setObjectName("ftp");
        btnRefresh->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/reload.png").scaled(24, 24)));
        tool->addWidget(btnRefresh);

        QPushButton *btnBrowser = new QPushButton(localArea);
        btnBrowser->setObjectName("ftp");
        btnBrowser->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/folder.png").scaled(24, 24)));
        QObject::connect(btnBrowser, SIGNAL(clicked(bool)), this, SLOT(onLocalBrowserButtonClicked()));
        tool->addWidget(btnBrowser);
        QLineEdit *edit = new QLineEdit(localArea);
        tool->addWidget(edit);
        edit->setReadOnly(true);
        m_localEdit = edit;
        m_btnHome = btnHome;
        m_btnEnter = btnEnter;
        m_btnReturn = btnReturn;

        QTreeView *local = new QTreeView(localArea);
        local->setObjectName("ftp");
        local->setIndentation(0);
        QKxFtpLocalModel* model = new QKxFtpLocalModel(local);
        local->setModel(model);
        layout->addWidget(local);
        m_local = local;
        m_modelLocal = model;
        QObject::connect(model, SIGNAL(modelReset()), this, SLOT(onLocalPathReset()), Qt::QueuedConnection);
        model->setHome();

        QHeaderView *hdr = local->header();
        local->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(local, SIGNAL(clicked(QModelIndex)), this, SLOT(onLocalItemClicked(QModelIndex)));
        QObject::connect(local, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onLocalItemDbClicked(QModelIndex)));
        QObject::connect(local, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onLocalContextMenuRequested(QPoint)));
    }
    {
        QVBoxLayout *layout = new QVBoxLayout(remoteArea);
        layout->setSpacing(3);
        remoteArea->setLayout(layout);
        layout->addWidget(new QLabel(tr("Remote folder"), remoteArea));
        QHBoxLayout *tool = new QHBoxLayout(remoteArea);
        layout->addLayout(tool);

        QPushButton *btnHome = new QPushButton(remoteArea);
        QObject::connect(btnHome, SIGNAL(clicked(bool)), this, SLOT(onRemoteHomeButtonClicked()));
        btnHome->setObjectName("ftp");
        btnHome->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/home.png").scaled(24, 24)));
        tool->addWidget(btnHome);
        QPushButton *btnReturn = new QPushButton(remoteArea);
        QObject::connect(btnReturn, SIGNAL(clicked(bool)), this, SLOT(onRemoteBackButtonClicked()));
        btnReturn->setObjectName("ftp");
        btnReturn->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/back.png").scaled(24, 24)));
        tool->addWidget(btnReturn);
        QPushButton *btnEnter = new QPushButton(remoteArea);
        QObject::connect(btnEnter, SIGNAL(clicked(bool)), this, SLOT(onRemoteForwardButtonClicked()));
        btnEnter->setObjectName("ftp");
        btnEnter->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/forward.png").scaled(24, 24)));
        tool->addWidget(btnEnter);

        QPushButton *btnRefresh = new QPushButton(remoteArea);
        QObject::connect(btnRefresh, SIGNAL(clicked(bool)), this, SLOT(onRemoteRefreshButtonClicked()));
        btnRefresh->setObjectName("ftp");
        btnRefresh->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/reload.png").scaled(24, 24)));
        tool->addWidget(btnRefresh);

        QPushButton *btnCopy = new QPushButton(remoteArea);
        btnCopy->setObjectName("ftp");
        btnCopy->setIcon(QIcon(QPixmap(":/kxftp/resource/skin/copy.png").scaled(24, 24)));
        QObject::connect(btnCopy, SIGNAL(clicked(bool)), this, SLOT(onRemoteCopyButtonClicked()));
        tool->addWidget(btnCopy);
        QLineEdit *edit = new QLineEdit(remoteArea);
        tool->addWidget(edit);
        edit->setReadOnly(true);
        m_btnHome2 = btnHome;
        m_btnEnter2 = btnEnter;
        m_btnReturn2 = btnReturn;
        m_remoteEdit = edit;


        QTreeView *remote = new QTreeView(remoteArea);
        remote->setObjectName("ftp");
        remote->setIndentation(0);
        layout->addWidget(remote);
        QKxFtpRemoteModel* model = new QKxFtpRemoteModel(remote);
        remote->setModel(model);
        QObject::connect(model, SIGNAL(modelReset()), this, SLOT(onRemotePathReset()), Qt::QueuedConnection);
        m_remote = remote;
        m_modelRemote = model;

        remote->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(remote, SIGNAL(clicked(QModelIndex)), this, SLOT(onRemoteItemClicked(QModelIndex)));
        QObject::connect(remote, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRemoteItemDbClicked(QModelIndex)));
        QObject::connect(remote, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRemoteContextMenuRequested(QPoint)));
    }
    {
        QVBoxLayout *layout = new QVBoxLayout(transferArea);
        layout->setSpacing(3);
        transferArea->setLayout(layout);
        layout->addWidget(new QLabel(tr("File transfer"), transferArea));
        QTreeView *transfer = new QTreeView(transferArea);
        transfer->setObjectName("ftp");
        transfer->setItemDelegate(new QKxProgress(transfer));
        QKxFtpTransferModel *model = new QKxFtpTransferModel(m_pathTask, m_name, transfer);
        transfer->setModel(model);
        QHeaderView *hdr = transfer->header();
        hdr->resizeSection(0, 40);
        hdr->setSectionResizeMode(0, QHeaderView::Fixed);
        hdr->resizeSection(1, 40);
        hdr->setSectionResizeMode(1, QHeaderView::Fixed);
        hdr->resizeSection(2, 150);
        hdr->setSectionResizeMode(2, QHeaderView::Fixed);
        hdr->setSectionResizeMode(3, QHeaderView::Fixed);
        hdr->setSectionResizeMode(4, QHeaderView::Stretch);
        hdr->setSectionResizeMode(5, QHeaderView::Stretch);
        m_modelTransfer = model;

        layout->addWidget(transfer);
        m_transfer = transfer;
        transfer->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(transfer, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onTransferContextMenuRequested(QPoint)));
        QObject::connect(model, SIGNAL(taskUpdate(QModelIndex)), this, SLOT(onTaskUpdate(QModelIndex)));
    }
}

QKxFtpTransferWidget::~QKxFtpTransferWidget()
{
    if(m_modelRemote) {
        m_modelRemote->deleteLater();
    }
    if(m_modelLocal) {
        m_modelLocal->deleteLater();
    }
    if(m_modelTransfer) {
        m_modelTransfer->deleteLater();
    }
    if(m_ftpRequest) {
        m_ftpRequest->deleteLater();
    }
}

void QKxFtpTransferWidget::setRequest(QKxFtpRequest *ftp)
{
    if(m_ftpRequest) {
        m_ftpRequest->deleteLater();
    }

    m_ftpRequest = ftp;
    m_modelRemote->setFtpRequest(m_ftpRequest);
    m_modelTransfer->setFtpRequest(m_ftpRequest);
    QObject::connect(m_ftpRequest, SIGNAL(listResult(qint8,QByteArray,QByteArrayList)), this, SLOT(onListFilesResult(qint8,QByteArray,QByteArrayList)));
    QObject::connect(m_ftpRequest, SIGNAL(mkdirResult(qint8,QByteArray)), this, SLOT(onMkdirResult(qint8,QByteArray)));
    QObject::connect(m_ftpRequest, SIGNAL(entryInfoListResult(qint8,QByteArray,QVariantList)), this, SLOT(onRemoteEntryInfoListResult(qint8,QByteArray,QVariantList)));

}

void QKxFtpTransferWidget::onLocalPathReset()
{
    QString path = m_modelLocal->path();
    m_btnHome->setEnabled(!path.isEmpty());
    m_btnReturn->setEnabled(!path.isEmpty());
    m_btnEnter->setEnabled(true);
    m_localEdit->setText(path);
}

void QKxFtpTransferWidget::onLocalHomeButtonClicked()
{
    m_modelLocal->setHome();
}

void QKxFtpTransferWidget::onLocalBackButtonClicked()
{
    QString path = m_modelLocal->path();
    if(path.isEmpty()) {
        return;
    }
    QFileInfo fi(path);
    if(fi.isRoot()) {
        m_modelLocal->setHome();
    }else {
        QString path = fi.absolutePath();
        m_modelLocal->setPath(path);
    }
}

void QKxFtpTransferWidget::onLocalForwardButtonClicked()
{
    QModelIndex idx = m_local->currentIndex();
    QFileInfo fi = m_modelLocal->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_modelLocal->setPath(path);
    }
}

void QKxFtpTransferWidget::onLocalRefreshButtonClicked()
{
    onLocalFileRefresh();
}

void QKxFtpTransferWidget::onLocalBrowserButtonClicked()
{
    QString path = m_localEdit->text();
    QString url = "file:///"+path;
    QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void QKxFtpTransferWidget::onLocalItemClicked(const QModelIndex &idx)
{
    if(idx.isValid()) {
        QFileInfo fi = m_modelLocal->fileInfo(idx);
        m_btnEnter->setEnabled(fi.isDir());
    }
}

void QKxFtpTransferWidget::onLocalItemDbClicked(const QModelIndex &idx)
{
    QFileInfo fi = m_modelLocal->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_modelLocal->setPath(path);
    }
}

void QKxFtpTransferWidget::onLocalContextMenuRequested(const QPoint &pos)
{
    QTreeView *viewer = qobject_cast<QTreeView*>(sender());
    QModelIndex idx = viewer->indexAt(pos);
    onLocalItemClicked(idx);
    QMenu menu(this);
    QFileInfo fi = m_modelLocal->fileInfo(idx);
    if(idx.isValid()) {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/upload.png"), tr("Upload"), this, SLOT(onLocalFileUpload()));
        tmp->setProperty("index", idx);
    }
    QString path = m_localEdit->text();
    if(!path.isEmpty()) {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/folder.png"), tr("Create Directory"), this, SLOT(onLocalCreateDirectory()));
        tmp->setProperty("index", idx);
    }
    {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/login.png"), tr("Browser Directory"), this, SLOT(onLocalBrowserDirectory()));
        tmp->setProperty("index", idx);
    }
    {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/reload.png"), tr("Refresh"), this, SLOT(onLocalFileRefresh()));
        tmp->setProperty("index", idx);
    }
    menu.exec(QCursor::pos());
}

void QKxFtpTransferWidget::onLocalCreateDirectory()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QInputDialog input(this);
    input.setWindowFlags(input.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    input.setMinimumWidth(350);
    input.setWindowTitle(tr("directory name"));
    input.setLabelText(tr("Please input a directory name"));
    int err = input.exec();
    if(err == 0) {
        return;
    }
    QString name = input.textValue();
    if(name.isEmpty()) {
        QMessageBox::information(this, tr("information"), tr("the new directory name should be empty!"));
        return;
    }
    QString path = m_localEdit->text();
    if(path.isEmpty()) {
        return;
    }
    QDir d(path);
    if(d.mkdir(name)) {
        m_modelLocal->reload();
    }

}

void QKxFtpTransferWidget::onLocalFileRefresh()
{
    m_modelLocal->reload();
}

void QKxFtpTransferWidget::onLocalBrowserDirectory()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        QFileInfo fi = m_modelLocal->fileInfo(idx);
        QString path = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
        QString url = "file:///"+path;
        QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
    }
}

void QKxFtpTransferWidget::onLocalFileUpload()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        QString path = m_remoteEdit->text();
        if(path.isEmpty()) {
            QMessageBox::information(this, tr("information"), tr("Please select a remote path to upload"));
            return;
        }
        QFileInfo fi = m_modelLocal->fileInfo(idx);
        if(fi.isDir()) {
            QStringList all;
            QString localPath = fi.absoluteFilePath();
            if(!listFiles(localPath, all, 500)) {
                QMessageBox::information(this, tr("information"), tr("The upload file number can't over 500"));
                return;
            }
            for(int i = 0; i < all.length(); i++) {
                QString path = all.at(i);
                all[i] = path.remove(localPath);
            }
            QString remotePath = m_remoteEdit->text();
            remotePath += "/" + fi.fileName();
            m_modelTransfer->addTask(all, localPath, remotePath, true);
            qDebug() << "fileUpload" << all;
        }else{
            QStringList all;
            QString localPath = fi.absolutePath();
            QString remotePath = m_remoteEdit->text();
            QString path = fi.absoluteFilePath();
            path = path.remove(localPath);
            all.append(path);
            m_modelTransfer->addTask(all, localPath, remotePath, true);
            qDebug() << "fileUpload" << fi.absoluteFilePath();
        }
    }
}

void QKxFtpTransferWidget::onBrowserLocalDirectory()
{
    QString path = m_localEdit->text();
    if(path.isEmpty()) {
        QMessageBox::information(this, tr("tip"), tr("Please select a directory to open."));
        return;
    }
    QString url = "file:///"+path;
    QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void QKxFtpTransferWidget::onRemotePathReset()
{
    QString path = m_modelRemote->path();
    m_btnHome2->setEnabled(!path.isEmpty());
    m_btnReturn2->setEnabled(!path.isEmpty());
    m_btnEnter2->setEnabled(true);
    m_remoteEdit->setText(path);
}

void QKxFtpTransferWidget::onRemoteHomeButtonClicked()
{
    if(m_ftpRequest) {
        m_ftpRequest->entryInfoList("");
    }
}

void QKxFtpTransferWidget::onRemoteBackButtonClicked()
{
    QString path = m_modelRemote->path();
    if(path.isEmpty() || path == "/") {
        m_ftpRequest->entryInfoList("");
    }else{
        QString tmp = QDir::cleanPath(path+"/");
        int ipos = tmp.lastIndexOf('/');
        if(ipos <= 0) {
            return;
        }
        tmp = tmp.mid(0, ipos+1);
        if(tmp == path) {
            // root.
            m_ftpRequest->entryInfoList("");
        }else{
            m_ftpRequest->entryInfoList(tmp.toUtf8());
        }
    }
}

void QKxFtpTransferWidget::onRemoteForwardButtonClicked()
{
    QModelIndex idx = m_remote->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    QKxFtpRemoteModel::FileInfo fi = m_modelRemote->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_ftpRequest->entryInfoList(path.toUtf8());
    }
}

void QKxFtpTransferWidget::onRemoteRefreshButtonClicked()
{
    onRemoteFileRefresh();
}

void QKxFtpTransferWidget::onRemoteCopyButtonClicked()
{
    QClipboard *clip = QGuiApplication::clipboard();
    QString path = m_remoteEdit->text();
    clip->setText(path);
}

void QKxFtpTransferWidget::onRemoteItemClicked(const QModelIndex &idx)
{
    if(idx.isValid()) {
        QKxFtpRemoteModel::FileInfo fi = m_modelRemote->fileInfo(idx);
        m_btnEnter2->setEnabled(fi.isDir());
    }
}

void QKxFtpTransferWidget::onRemoteItemDbClicked(const QModelIndex &idx)
{
    QKxFtpRemoteModel::FileInfo fi = m_modelRemote->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_ftpRequest->entryInfoList(path.toUtf8());
    }
}

void QKxFtpTransferWidget::onRemoteContextMenuRequested(const QPoint &pos)
{
    QTreeView *viewer = qobject_cast<QTreeView*>(sender());
    QModelIndex idx = viewer->indexAt(pos);
    onRemoteItemClicked(idx);
    QMenu menu(this);
    bool hasMenu = false;
    if(idx.isValid()) {
        // only device in account can download file from remote device.
        // for safety.
        // prevent from bad men steal resource from normal device.
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/download.png"), tr("Download"), this, SLOT(onRemoteFileDownload()));
        tmp->setProperty("index", idx);
        hasMenu = true;
    }
    QString path = m_remoteEdit->text();
    if(!path.isEmpty()) {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/folder.png"), tr("Create Directory"), this, SLOT(onRemoteCreateDirectory()));
        tmp->setProperty("index", idx);
        hasMenu = true;
    }
    {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/reload.png"), tr("Refresh"), this, SLOT(onRemoteFileRefresh()));
        tmp->setProperty("index", idx);
    }
    menu.exec(QCursor::pos());
}

void QKxFtpTransferWidget::onRemoteCreateDirectory()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QInputDialog input(this);
    input.setWindowFlags(input.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    input.setMinimumWidth(350);
    input.setWindowTitle(tr("Directory name"));
    input.setLabelText(tr("Please input a directory name"));
    int err = input.exec();
    if(err == 0) {
        return;
    }
    QString name = input.textValue();
    if(name.isEmpty()) {
        QMessageBox::information(this, tr("information"), tr("the new directory name should be empty!"));
        return;
    }
    QString path = m_remoteEdit->text();
    if(path.isEmpty()) {
        return;
    }
    m_ftpRequest->mkdir(path.toUtf8(), name.toUtf8());
}

void QKxFtpTransferWidget::onRemoteFileRefresh()
{
    QString path = m_modelRemote->path();
    if(m_ftpRequest) {
        m_ftpRequest->entryInfoList(path.toUtf8());
    }
}

void QKxFtpTransferWidget::onRemoteFileDownload()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        QString path = m_localEdit->text();
        if(path.isEmpty()) {
            QMessageBox::information(this, tr("information"), tr("Please select a local path to save download files"));
            return;
        }
        QKxFtpRemoteModel::FileInfo fi = m_modelRemote->fileInfo(idx);
        if(fi.isDir()) {
            m_ftpRequest->listFiles(fi.absoluteFilePath().toUtf8());
        }else{
            QStringList all;
            QString remotePath = fi.absolutePath();
            QString localPath = m_localEdit->text();
            QString path = fi.absoluteFilePath();
            path = path.remove(remotePath);
            all.append(path);
            m_modelTransfer->addTask(all, localPath, remotePath, false);
            qDebug() << "fileUpload" << fi.absoluteFilePath();
        }
    }
}

void QKxFtpTransferWidget::onRemoteDirectoryReload()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        emit reload();
    }
}

void QKxFtpTransferWidget::onRemoteEntryInfoListResult(qint8 err, const QByteArray &path, const QVariantList &files)
{
    qDebug() << err << path;
    if(err == 0) {
        m_modelRemote->setEntryInfoList(path, files);
    }
}

void QKxFtpTransferWidget::onListFilesResult(qint8 err, const QByteArray &path, const QByteArrayList &files)
{
    if(err == 0) {
        QString localPath = m_localEdit->text();
        QString tmp = QDir::cleanPath(path);
        int pos = tmp.lastIndexOf('/');
        if(pos > 0) {
            localPath += tmp.mid(pos);
        }
        QStringList all;
        for(int i = 0; i < files.length(); i++) {
            all.append(files.at(i));
        }
        m_modelTransfer->addTask(all, localPath, path, false);
    }else{
        QMessageBox::information(this, tr("information"), tr("The number of selected files cannot over 500."));
    }
}

void QKxFtpTransferWidget::onMkdirResult(qint8 err, const QByteArray &msg)
{
    if(err == 0) {
        QString path = m_modelRemote->path();
        m_ftpRequest->entryInfoList(path.toUtf8());
    }
}

void QKxFtpTransferWidget::onTransferContextMenuRequested(const QPoint &pos)
{
    QTreeView *viewer = qobject_cast<QTreeView*>(sender());
    QModelIndex idx = viewer->indexAt(pos);
    int cnt = m_modelTransfer->taskCount();
    if(cnt <= 0) {
        return;
    }
    QMenu menu(this);
    if(idx.isValid()) {
        bool canStart = m_modelTransfer->first(QKxFtpTransferModel::ETS_Transfer) < 0;
        QKxFtpTransferModel::ETaskStatus status = m_modelTransfer->taskStatus(idx);
        if(status == QKxFtpTransferModel::ETS_Pause) {
            QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/start.png"), tr("Start"), this, SLOT(onTransferStart()));
            tmp->setProperty("index", idx);
        }
        if(status == QKxFtpTransferModel::ETS_Ready || status == QKxFtpTransferModel::ETS_Transfer) {
            if(canStart) {
                QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/start.png"), tr("Start"), this, SLOT(onTransferStart()));
                tmp->setProperty("index", idx);
            }else{
                QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/pause.png"), tr("Stop"), this, SLOT(onTransferStop()));
                tmp->setProperty("index", idx);
            }
        }
        {
            QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/reload.png"), tr("Restart"), this, SLOT(onTransferRestart()));
            tmp->setProperty("index", idx);
        }
        {
            QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/remove.png"), tr("Remove"), this, SLOT(onTransferRemove()));
            tmp->setProperty("index", idx);
        }
    }
    if(m_modelTransfer->startCount() > 0) {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/stopall.png"), tr("Stop all"), this, SLOT(onTransferStopAll()));
        tmp->setProperty("index", idx);
    }
    if(m_modelTransfer->stopCount() > 0) {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/startall.png"), tr("Start all"), this, SLOT(onTransferStartAll()));
        tmp->setProperty("index", idx);
    }
    {
        QAction *tmp = menu.addAction(QIcon(":/kxftp/resource/skin/removeall.png"), tr("Remove all"), this, SLOT(onTransferRemoveAll()));
        tmp->setProperty("index", idx);
    }
    menu.exec(QCursor::pos());
}

void QKxFtpTransferWidget::onTransferStart()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        m_modelTransfer->start(idx);
    }
}

void QKxFtpTransferWidget::onTransferRestart()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        m_modelTransfer->restart(idx);
    }
}

void QKxFtpTransferWidget::onTransferStop()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        m_modelTransfer->stop(idx);
    }
}

void QKxFtpTransferWidget::onTransferStartAll()
{
    m_modelTransfer->startAll();
}

void QKxFtpTransferWidget::onTransferStopAll()
{
    m_modelTransfer->stopAll();
}

void QKxFtpTransferWidget::onTransferRemove()
{
    QAction *tmp = qobject_cast<QAction*>(sender());
    QModelIndex idx = tmp->property("index").toModelIndex();
    if(idx.isValid()) {
        m_modelTransfer->remove(idx);
    }
}

void QKxFtpTransferWidget::onTransferRemoveAll()
{
    m_modelTransfer->removeAll();
}

void QKxFtpTransferWidget::onTaskUpdate(const QModelIndex& idx)
{
    QRect rt = m_transfer->visualRect(idx);
    if(rt.isValid()) {
        QWidget *w = m_transfer->viewport();
        w->update();
    }
}

void QKxFtpTransferWidget::resizeEvent(QResizeEvent *ev)
{
    QSize sz = ev->size();
    QWidget::resizeEvent(ev);
}

void QKxFtpTransferWidget::showEvent(QShowEvent *event)
{
    if(m_ftpRequest != nullptr) {
        m_ftpRequest->entryInfoList("");
    }
    QWidget::showEvent(event);
}

void QKxFtpTransferWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

bool QKxFtpTransferWidget::listFiles(const QString &path, QStringList& all, int maxCount)
{
    QFileInfo fi(path);
    if(fi.isDir()) {
        QDir d(fi.absoluteFilePath());
        QFileInfoList fil = d.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
        for(int i = 0; i < fil.length(); i++) {
            QFileInfo fi = fil.at(i);
            if(!listFiles(fi.absoluteFilePath(), all, maxCount)) {
                return false;
            }
        }
    }else{
        all.append(fi.absoluteFilePath());
    }
    return all.length() < maxCount;
}

