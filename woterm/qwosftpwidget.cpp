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

#include "qwosftpwidget.h"
#include "ui_qwosftpwidget.h"
#include "qwossh.h"
#include "qwosftplistmodel.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwosshconf.h"
#include "qwosftpdownmgrmodel.h"
#include "qkxutils.h"
#include "qwotreeview.h"
#include "qwosftpnamedialog.h"
#include "qwosftptransferwidget.h"
#include "qwosetting.h"
#include "qwoloadingwidget.h"
#include "qwoutils.h"
#include "qwoshower.h"
#include "qwomainwindow.h"

#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

QWoSftpWidget::QWoSftpWidget(const QString &target, int gid, QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::QWoSftpWidget)
    ,m_gid(gid)
    ,m_target(target)
    ,m_savePassword(false)
    ,m_bexit(false)
{
    ui->setupUi(this);

    //m_loading = new QWoLoadingWidget(":/woterm/resource/skin/loading.png", this);
    m_loading = new QWoLoadingWidget(QColor("#1296DB"), this);
    m_transfer = new QWoSftpTransferWidget(this);
    QObject::connect(m_transfer, SIGNAL(abort()), this, SLOT(onTransferAbort()));

    m_model = new QWoSftpListModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    QVBoxLayout *layout = new QVBoxLayout(ui->frame);
    ui->frame->setLayout(layout);
    ui->frame->setFrameShape(QFrame::NoFrame);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_list = new QWoTreeView(ui->frame);
    m_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setIconSize(QSize(24, 24));
    layout->addWidget(m_list);
    m_list->installEventFilter(this);
    m_list->setModel(m_proxyModel);    
    //m_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QObject::connect(m_list, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onListItemDoubleClicked(const QModelIndex&)));
    QObject::connect(m_list, SIGNAL(itemChanged(const QModelIndex&)), this, SLOT(onListCurrentItemChanged(const QModelIndex&)));
    QObject::connect(m_list, SIGNAL(returnKeyPressed()), this, SLOT(onListReturnKeyPressed()));

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    QObject::connect(m_model, SIGNAL(modelReset()), this, SLOT(onResetModel()), Qt::QueuedConnection);
}

QWoSftpWidget::~QWoSftpWidget()
{
    delete ui;

    m_sftp->abort();
    QWoSshFactory::instance()->release(m_sftp);
}

void QWoSftpWidget::openHome()
{
    m_sftp->openDir();
}

void QWoSftpWidget::openDir(const QString &path)
{
    m_sftp->openDir(path);
}

void QWoSftpWidget::init(QObject *obj, const QString& type)
{
    if(type == "dialog") {
        QWidget *dlg = qobject_cast<QWidget*>(obj);
        if(dlg) {
            dlg->setParent(this);
        }
    }
}

void QWoSftpWidget::mkDir(const QString &path, const QString &name, int mode)
{
    m_sftp->mkDir(path, name, mode);
}

void QWoSftpWidget::rmDir(const QString &path, const QString &name)
{
    m_sftp->rmDir(path, name);
}

void QWoSftpWidget::unlink(const QString &path, const QString &name)
{
    m_sftp->unlink(path, name);
}

int QWoSftpWidget::textWidth(const QString &txt, const QFont &ft)
{
    QFontMetricsF fm(ft);
    QSizeF sz = fm.size(Qt::TextSingleLine, txt);
    return sz.width();
}

void QWoSftpWidget::download(const QString &remote, const QString &local)
{
    m_sftp->download(remote, local);
}

void QWoSftpWidget::upload(const QString &local, const QString &remote)
{
    m_sftp->upload(local, remote);
}


QList<QString> QWoSftpWidget::collectUnsafeCloseMessage()
{
    return QList<QString>();
}

void QWoSftpWidget::onConnectionStart()
{
    m_loading->show();
}

void QWoSftpWidget::onConnectionFinished(bool ok)
{
    m_loading->hide();
    if(ok) {
        QString path = m_model->path();
        if(path.isEmpty()) {
            path = ".";
        }
        openDir(path);
    }else{
        onFinishArrived(-1);
    }
}

void QWoSftpWidget::onProgress(int type, int v)
{
    m_transfer->setVisible(v > 0 && v < 100);
    m_transfer->progress(v);
}

void QWoSftpWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(this);
        QObject::connect(m_passInput, SIGNAL(result(const QString&,bool)), this, SLOT(onPasswordInputResult(const QString&,bool)));
    }
    if(m_passInput->isVisible()) {
        return;
    }
    m_passInput->reset(title, prompt, echo);
    m_passInput->adjustSize();
    m_passInput->showNormal();
    m_passInput->raise();
    QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
}

void QWoSftpWidget::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    QSize sz = ev->size();
    //qDebug() << "resizeEvent" << objectName() << sz << ev->oldSize();
    if(m_mask) {
        m_mask->setGeometry(0, 0, sz.width(), sz.height());
    }
    if(m_transfer) {
        m_transfer->setGeometry(0, 0, sz.width(), sz.height());
    }
    if(m_loading) {
        m_loading->setGeometry(0, 0, sz.width(), sz.height());
    }
    if(m_passInput) {
        QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
    }
}

bool QWoSftpWidget::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type t = ev->type();
    if(obj == m_list) {
        if(t == QEvent::ContextMenu) {
            return handleListContextMenu(dynamic_cast<QContextMenuEvent*>(ev));
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QWoSftpWidget::reconnect()
{
    if(m_passInput) {
        m_passInput->deleteLater();
    }
    if(m_sftp) {
        QWoSshFactory::instance()->release(m_sftp);
    }
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(dirOpen(const QString&, const QVariantList&)), m_model, SLOT(onDirOpen(const QString&, const QVariantList&)));
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(errorArrived(const QByteArray&)), this, SLOT(onErrorArrived(const QByteArray&)));
    QObject::connect(m_sftp, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SLOT(onPasswordArrived(const QString&,const QByteArray&)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));

    QObject::connect(m_sftp, SIGNAL(commandStart(int)), this, SLOT(onCommandStart(int)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int)), this, SLOT(onCommandFinish(int)));
    QObject::connect(m_sftp, SIGNAL(progress(int,int)), this, SLOT(onProgress(int,int)));

    m_sftp->start(m_target, m_gid);
    openDir(".");
    m_loading->show();
}

bool QWoSftpWidget::handleListContextMenu(QContextMenuEvent *ev)
{
    QItemSelectionModel *model = m_list->selectionModel();
    QModelIndexList idxs = model->selectedIndexes();

    QMenu menu(this);
    if(idxs.isEmpty()) {
        menu.addAction(QIcon(":/woterm/resource/skin/back3.png"), tr("Back"), this, SLOT(onMenuReturnTopDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Refresh"), this, SLOT(onMenuReloadDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/home.png"), tr("Home Directory"), this, SLOT(onMenuGoHomeDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/mkdir.png"), tr("Create Directory"), this, SLOT(onMenuCreateDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/upload2.png"), tr("Upload"), this, SLOT(onMenuUpload()));
    }else{
        QModelIndex idx = idxs.at(0);
        FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
        menu.addAction(QIcon(":/woterm/resource/skin/back3.png"), tr("Back"), this, SLOT(onMenuReturnTopDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/reload.png"), tr("Refresh"), this, SLOT(onMenuReloadDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/home.png"), tr("Home Directory"), this, SLOT(onMenuGoHomeDirectory()));
        menu.addAction(QIcon(":/woterm/resource/skin/mkdir.png"), tr("Create Directory"), this, SLOT(onMenuCreateDirectory()));
        if(fi.type == "d") {
            menu.addAction(QIcon(":/woterm/resource/skin/rmdir.png"), tr("Remove Directory"), this, SLOT(onMenuRemoveDirectory()));
            menu.addAction(QIcon(":/woterm/resource/skin/enter.png"), tr("Enter"), this, SLOT(onMenuEnterDirectory()));
        }else if(fi.type == "-"){
            menu.addAction(QIcon(":/woterm/resource/skin/rmfile.png"), tr("Remove File"), this, SLOT(onMenuRemoveFile()));
            menu.addAction(QIcon(":/woterm/resource/skin/download2.png"), tr("Download"), this, SLOT(onMenuDownload()));
        }else if(fi.type == "l") {
            menu.addAction(QIcon(":/woterm/resource/skin/enter3.png"), tr("Try Enter"), this, SLOT(onMenuTryEnterDirectory()));
        }
        menu.addAction(QIcon(":/woterm/resource/skin/upload2.png"), tr("Upload"), this, SLOT(onMenuUpload()));
    }
    menu.addAction(QIcon(":/woterm/resource/skin/multiplex.png"), tr("New Session Multiplex"), this, SLOT(onNewSessionMultiplex()));
    menu.addAction(tr("Close"), this, SLOT(onForceToCloseThisSession()));
    menu.exec(ev->globalPos());
    return true;
}

void QWoSftpWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;
    if(m_mask && m_mask->isVisible()) {
        return;
    }

    if(m_mask == nullptr) {
        m_mask = new QWoTermMask(this);
        QObject::connect(m_mask, SIGNAL(aboutToClose(QCloseEvent*)), this, SLOT(onForceToCloseThisSession()));
        QObject::connect(m_mask, SIGNAL(reconnect()), this, SLOT(onSessionReconnect()));
    }
    m_mask->setGeometry(0, 0, width(), height());
    m_mask->show();
    m_loading->hide();
}

void QWoSftpWidget::onErrorArrived(const QByteArray &buf)
{
    QMessageBox::warning(this, tr("Error"), buf, QMessageBox::Ok);
}

void QWoSftpWidget::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    showPasswordInput(title, prompt, visible);
}

void QWoSftpWidget::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword) {
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QWoSftpWidget::onPasswordInputResult(const QString &pass, bool isSave)
{
    m_savePassword = isSave;
    if(m_sftp) {
        m_sftp->setInputResult(pass);
    }
}

void QWoSftpWidget::onSessionReconnect()
{
    reconnect();
}

void QWoSftpWidget::onForceToCloseThisSession()
{
    deleteLater();
}

void QWoSftpWidget::onListItemDoubleClicked(const QModelIndex &item)
{
    FileInfo fi = item.data(ROLE_FILEINFO).value<FileInfo>();
    qDebug() << "server:" << fi.name;
    if(fi.type == "d" || fi.type == "l") {
        QString path = m_model->path() + "/" + fi.name;
        openDir(path);
    }
}

void QWoSftpWidget::onListCurrentItemChanged(const QModelIndex &item)
{
    FileInfo fi = item.data(ROLE_FILEINFO).value<FileInfo>();
    qDebug() << "server:" << fi.name;
}

void QWoSftpWidget::onListReturnKeyPressed()
{
    QModelIndex idx = m_list->currentIndex();
    onListItemDoubleClicked(idx);}

void QWoSftpWidget::onMenuReturnTopDirectory()
{
    QString path = m_model->path() + "/..";
    openDir(path);
}

void QWoSftpWidget::onMenuGoHomeDirectory()
{
    openDir(".");
}

void QWoSftpWidget::onMenuReloadDirectory()
{
    QString path = m_model->path();
    openDir(path);
}

void QWoSftpWidget::onMenuCreateDirectory()
{
    QWoSftpNameDialog dlg("", this);
    dlg.exec();
    QString fileName = dlg.result();
    if(fileName.isEmpty()) {
        return;
    }
    bool isPrivate = dlg.isPrivate();
    QString path = m_model->path();
    mkDir(path, fileName, isPrivate ? 0x1C0 : 0x1FF);
}

void QWoSftpWidget::onMenuRemoveDirectory()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    QString path = m_model->path();
    rmDir(path, fi.name);
}

void QWoSftpWidget::onMenuRemoveFile()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    QString path = m_model->path();
    unlink(path, fi.name);
}

void QWoSftpWidget::onMenuEnterDirectory()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    QString path = m_model->path();
    openDir(path + "/" + fi.name);
}

void QWoSftpWidget::onMenuTryEnterDirectory()
{
    onMenuEnterDirectory();
}

void QWoSftpWidget::onMenuDownload()
{
    QModelIndex idx = m_list->currentIndex();
    if(!idx.isValid()) {
        return;
    }

    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    QString path = m_model->path();
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QString fileDst = pathLast + "/" + fi.name;
    fileDst = QFileDialog::getSaveFileName(this, tr("SaveFile"), fileDst);
    qDebug() << "fileDst" << fileDst;
    if(fileDst.isEmpty()) {
        return;
    }
    int i = fileDst.lastIndexOf("/");
    if(i > 0) {
        QWoSetting::setValue("zmodem/lastPath", fileDst.left(i));
    }
    QString fileSrc = QDir::cleanPath(path + "/" + fi.name);
    if(QFile::exists(fileDst)) {
        if(QMessageBox::warning(this, tr("FileExist"),
                                tr("has the same name in the target path. override it?"),
                                QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
        QFile::remove(fileDst);
    }
    m_transfer->setTip(fileSrc + "->" + fileDst);
    download(fileSrc, fileDst);
}

void QWoSftpWidget::onMenuUpload()
{
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QString fileSrc = QFileDialog::getOpenFileName(this, tr("Select File"), pathLast);
    if(fileSrc.isEmpty()) {
        return;
    }
    QString name = fileSrc;
    int idx = fileSrc.lastIndexOf("/");
    if(idx > 0) {
        name = fileSrc.mid(idx);
    }
    QString path = m_model->path();
    QString fileDst = QDir::cleanPath(path + name);
    m_transfer->setTip(fileSrc + "->" + fileDst);
    upload(fileSrc, fileDst);
}

void QWoSftpWidget::onNewSessionMultiplex()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSftp(m_target, m_gid);
}

void QWoSftpWidget::onResetModel()
{
    for(int i = 0; i < 5;i++) {
        m_list->resizeColumnToContents(i);
    }
}

void QWoSftpWidget::onAdjustPosition()
{
    if(m_passInput) {
        QSize sz = m_passInput->minimumSize();
        if(sz.width() == 0 || sz.height() == 0) {
            sz = m_passInput->size();
        }
        QRect rt(0, 0, sz.width(), sz.height());
        rt.moveCenter(QPoint(width() / 2, height() / 2));
        m_passInput->setGeometry(rt);
    }
}

void QWoSftpWidget::onTransferAbort()
{
    m_sftp->abort();
}

void QWoSftpWidget::onCommandStart(int t)
{
    if(t == 15 || t == 14) {
        m_transfer->show();
        m_loading->hide();
    }else{
        m_transfer->hide();
        m_loading->show();
    }
}

void QWoSftpWidget::onCommandFinish(int t)
{
    m_transfer->hide();
    m_loading->hide();
}
