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
#include "qwosftpremotemodel.h"
#include "qwotermmask.h"
#include "qwopasswordinput.h"
#include "qwosshconf.h"
#include "qkxutils.h"
#include "qwosftptreeview.h"
#include "qwosftpnamedialog.h"
#include "qwosftptransferwidget.h"
#include "qwosetting.h"
#include "qwoloadingwidget.h"
#include "qwoutils.h"
#include "qkxver.h"
#include "qwoshower.h"
#include "qwomainwindow.h"
#include "qwosftpremotemodel.h"
#include "qwosftplocalmodel.h"
#include "qwosftppermissiondialog.h"
#include "qwosftprenamedialog.h"
#include "qkxmessagebox.h"
#include "qkxbuttonassist.h"
#include "qwosftpmovefiledialog.h"

#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QMenu>
#include <QFileDialog>
#include <QDir>
#include <QSplitter>
#include <QBoxLayout>
#include <QInputDialog>
#include <QDesktopServices>
#include <QMimeData>
#include <QDropEvent>
#include <QPushButton>
#include <QProcess>

#define SYNC_FOLLOW_FLAG    ("syncFollow")

QWoSftpWidget::QWoSftpWidget(const QString &target, int gid, bool assist, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoSftpWidget)
    , m_gid(gid)
    , m_isUltimate(true)
    , m_target(target)
    , m_savePassword(false)
    , m_bexit(false)
    , m_bAssist(assist)
{
    setAttribute(Qt::WA_StyledBackground, true);

    ui->setupUi(this);
    m_transfer = new QWoSftpTransferWidget(target, gid, m_isUltimate, this);
    m_transfer->installEventFilter(this);
    QObject::connect(m_transfer, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onTransferCommandStart(int,QVariantMap)));
    QObject::connect(m_transfer, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onTransferCommandFinish(int,QVariantMap)));

    m_transfer->hide();
    ui->btnTransfer->setVisible(false);
    if(assist) {
        ui->localArea->deleteLater();
    }else{
        QSplitter *root = new QSplitter(this);
        root->setHandleWidth(3);
        root->setOrientation(Qt::Horizontal);
        root->setContentsMargins(0, 0, 0, 0);

        QBoxLayout *ly = qobject_cast<QBoxLayout*>(layout());
        ly->addWidget(root);
        ly->removeWidget(ui->localArea);
        ly->removeWidget(ui->remoteArea);
        root->addWidget(ui->localArea);
        root->addWidget(ui->remoteArea);

        QVBoxLayout *layout = new QVBoxLayout(ui->localFrame);
        layout->setSpacing(0);
        layout->setMargin(0);
        ui->localFrame->setLayout(layout);
        ui->localFrame->setFrameShape(QFrame::NoFrame);

        m_local = new QWoSftpTreeView(ui->localFrame);
        m_local->setDragDropMode(QAbstractItemView::DropOnly);
        m_local->setAcceptDrops(true);
        m_local->setObjectName("localTreeView");
        m_local->viewport()->installEventFilter(this);
        m_local->viewport()->setObjectName("localViewPort");
        m_local->installEventFilter(this);
        m_local->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_local->setSelectionMode(m_isUltimate ?  QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
        m_local->setIconSize(QSize(20, 20));
        m_local->setSortingEnabled(true);
        m_local->sortByColumn(0, Qt::AscendingOrder);
        layout->addWidget(m_local);
        QWoSftpLocalModel* model = new QWoSftpLocalModel(this);
        m_proxyLocal = new QSortFilterProxyModel(this);
        m_proxyLocal->setSourceModel(model);
        m_proxyLocal->setSortRole(ROLE_MYSORT);
        m_localModel = model;
        m_local->setModel(m_proxyLocal);
        QObject::connect(model, SIGNAL(modelReset()), this, SLOT(onLocalResetModel()), Qt::QueuedConnection);
        m_local->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(m_local, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onLocalItemDoubleClicked(QModelIndex)));
        QObject::connect(m_local, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onLocalContextMenuRequested(QPoint)));
        QObject::connect(m_localModel, SIGNAL(pathChanged(QString)), this, SLOT(onLocalPathChanged(QString)));
        QObject::connect(m_local, SIGNAL(dropArrived(QList<QUrl>)), this, SLOT(onLocalDropArrived(QList<QUrl>)));

        QObject::connect(ui->btnLocalHome, SIGNAL(clicked()), this, SLOT(onLocalHomeButtonClicked()));
        QObject::connect(ui->btnLocalBack, SIGNAL(clicked()), this, SLOT(onLocalBackButtonClicked()));
        QObject::connect(ui->btnLocalForward, SIGNAL(clicked()), this, SLOT(onLocalForwardButtonClicked()));
        QObject::connect(ui->btnLocalReload, SIGNAL(clicked()), this, SLOT(onLocalReloadButtonClicked()));
        QObject::connect(ui->btnLocalBrowser, SIGNAL(clicked()), this, SLOT(onLocalBrowserButtonClicked()));

        QString pathLast = QWoSetting::value("sftp/lastPathLocal").toString();
        if(pathLast.isEmpty()) {
            model->setHome();
        }else{
            QFileInfo fi(pathLast);
            if(fi.isDir()) {
                model->setPath(pathLast);
            }else{
                model->setHome();
            }
        }
        QKxButtonAssist *my = new QKxButtonAssist("../private/skins/black/keyenter.png", ui->localPath);
        QObject::connect(ui->localPath, SIGNAL(returnPressed()), this, SLOT(onLocalPathReturnPressed()));
        QObject::connect(my, SIGNAL(clicked(int)), this, SLOT(onLocalPathReturnPressed()));
    }

    m_remoteModel = new QWoSftpRemoteModel(this);
    m_proxyRemote = new QSortFilterProxyModel(this);
    m_proxyRemote->setSourceModel(m_remoteModel);
    m_proxyRemote->setSortRole(ROLE_MYSORT);

    QVBoxLayout *layout = new QVBoxLayout(ui->remoteFrame);
    ui->remoteFrame->setLayout(layout);
    ui->remoteFrame->setFrameShape(QFrame::NoFrame);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_remote = new QWoSftpTreeView(ui->remoteFrame);    
    m_remote->setDragDropMode(QAbstractItemView::DropOnly);
    m_remote->setAcceptDrops(true);
    m_remote->viewport()->installEventFilter(this);
    m_remote->installEventFilter(this);
    m_remote->setObjectName("remoteTreeView");
    m_remote->viewport()->setObjectName("remoteViewPort");
    m_remote->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_remote->setSelectionMode(m_isUltimate ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
    m_remote->setIconSize(QSize(20, 20));
    m_remote->setSortingEnabled(true);
    m_remote->sortByColumn(0, Qt::AscendingOrder);
    m_remote->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_remote);
    m_remote->setModel(m_proxyRemote);
    m_loading = new QWoLoadingWidget(QColor(qRgb(18,150,219)), m_remote);

    QObject::connect(m_remote, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRemoteItemDoubleClicked(QModelIndex)));
    QObject::connect(m_remote, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRemoteContextMenuRequested(QPoint)));
    QObject::connect(m_remote, SIGNAL(dropArrived(QList<QUrl>)), this, SLOT(onRemoteDropArrived(QList<QUrl>)));
    QObject::connect(ui->btnRemoteHome, SIGNAL(clicked()), this, SLOT(onRemoteHomeButtonClicked()));
    QObject::connect(ui->btnRemoteBack, SIGNAL(clicked()), this, SLOT(onRemoteBackButtonClicked()));
    QObject::connect(ui->btnRemoteForward, SIGNAL(clicked()), this, SLOT(onRemoteForwardButtonClicked()));
    QObject::connect(ui->btnRemoteReload, SIGNAL(clicked()), this, SLOT(onRemoteReloadButtonClicked()));
    QObject::connect(m_remoteModel, SIGNAL(pathChanged(QString)), this, SLOT(onRemotePathChanged(QString)));
    QObject::connect(ui->btnTransfer, SIGNAL(clicked()), this, SLOT(onRemoteTransferButtonClicked()));

    if(m_isUltimate && assist) {
        ui->btnFollow->setProperty(SYNC_FOLLOW_FLAG, false);
        ui->btnFollow->setIcon(QIcon("../private/skins/black/select2.png"));
        ui->btnFollow->setToolTip(tr("The directory is out of sync with the terminal."));
        QObject::connect(ui->btnFollow, SIGNAL(clicked()), this, SLOT(onRemoteFollowButtonClicked()));
    }else{
        ui->btnFollow->setVisible(false);
    }

    {
        QKxButtonAssist *my = new QKxButtonAssist("../private/skins/black/keyenter.png", ui->remotePath);
        QObject::connect(ui->remotePath, SIGNAL(returnPressed()), this, SLOT(onRemotePathReturnPressed()));
        QObject::connect(my, SIGNAL(clicked(int)), this, SLOT(onRemotePathReturnPressed()));
    }

    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    QObject::connect(m_remoteModel, SIGNAL(modelReset()), this, SLOT(onRemoteResetModel()), Qt::QueuedConnection);
}

QWoSftpWidget::~QWoSftpWidget()
{
    delete ui;

    m_sftp->abort();
    QWoSshFactory::instance()->release(m_sftp);
}

void QWoSftpWidget::openHome()
{
    QVariantMap dm;
    dm.insert("path", "~");
    m_sftp->openDir("~", dm);
}

void QWoSftpWidget::openDir(const QStringList &paths)
{
    QVariantMap dm;
    dm.insert("paths", paths);
    m_sftp->openDir(paths, dm);
}

void QWoSftpWidget::openDir(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->openDir(path, dm);
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

void QWoSftpWidget::mkDir(const QString &path, int mode)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->mkDir(path, mode, dm);
}

void QWoSftpWidget::rmDir(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->rmDir(path, dm);
}

void QWoSftpWidget::unlink(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->unlink(path, dm);
}

int QWoSftpWidget::textWidth(const QString &txt, const QFont &ft)
{
    QFontMetricsF fm(ft);
    QSizeF sz = fm.size(Qt::TextSingleLine, txt);
    return sz.width();
}

bool QWoSftpWidget::canTransfer()
{
    if(!m_transfer->canAddTask()) {
        QKxMessageBox::message(this, tr("Reject request"), tr("Please wait until the current task is completed."), false);
        return false;
    }
    return true;
}

void QWoSftpWidget::release()
{
    if(m_sftp) {
        m_sftp->stop();
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}

QList<QString> QWoSftpWidget::collectUnsafeCloseMessage()
{
    return QList<QString>();
}

void QWoSftpWidget::onTransferAbort()
{
    //
}

void QWoSftpWidget::onConnectionStart()
{
    m_loading->show();
}

void QWoSftpWidget::onConnectionFinished(bool ok)
{
    m_loading->hide();
    if(!ok) {
        onFinishArrived(-1);
    }
}

void QWoSftpWidget::showPasswordInput(const QString &title, const QString &prompt, bool echo)
{
    if(m_passInput == nullptr) {
        m_passInput = new QWoPasswordInput(this);
        QObject::connect(m_passInput, SIGNAL(result(QString,bool)), this, SLOT(onPasswordInputResult(QString,bool)));
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
        QRect rt = m_transfer->rect();
        rt.moveCenter(QPoint(sz.width() / 2, sz.height() / 2));
        m_transfer->setGeometry(rt);
    }

    if(m_passInput) {
        QTimer::singleShot(0, this, SLOT(onAdjustPosition()));
    }
}

bool QWoSftpWidget::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type t = ev->type();
    //qDebug() << "eventFilter" << t << obj->objectName();
    if(m_local && obj == m_local->viewport()) {
        if(t == QEvent::MouseButtonPress) {
            QMouseEvent *me = (QMouseEvent*)ev;
            if(me->button() == Qt::LeftButton) {
                const QModelIndex& idx = m_local->indexAt(me->pos());
                if(!idx.isValid()) {
                    m_local->clearSelection();
                }
            }
        }
    } else if(obj == m_remote) {
        if(t == QEvent::Resize) {
            QResizeEvent *re = (QResizeEvent*)ev;
            QSize sz = re->size();
            if(m_loading) {
                m_loading->setGeometry(0, 0, sz.width(), sz.height());
            }
        }
    }else if(m_remote && obj == m_remote->viewport()) {
        if(t == QEvent::MouseButtonPress) {
            QMouseEvent *me = (QMouseEvent*)ev;
            if(me->button() == Qt::LeftButton) {
                const QModelIndex& idx = m_remote->indexAt(me->pos());
                if(!idx.isValid()) {
                    m_remote->clearSelection();
                }
            }
        }
    } else if(obj == m_transfer) {
        if(t == QEvent::Resize) {
            QResizeEvent *re = (QResizeEvent*)ev;
            QSize sz = re->size();
            if(m_transfer) {
                QRect rt = QRect(0, 0, sz.width(), sz.height());
                sz = this->size();
                rt.moveCenter(QPoint(sz.width() / 2, sz.height() / 2));
                m_transfer->setGeometry(rt);
            }
        }else if(t == QEvent::Show) {
            if(m_transfer) {
                QRect rt = m_transfer->rect();
                QSize sz = this->size();
                rt.moveCenter(QPoint(sz.width() / 2, sz.height() / 2));
                m_transfer->setGeometry(rt);
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QWoSftpWidget::handleLocalDragEnterEvent(QDropEvent *de)
{
    const QMimeData *md = de->mimeData();
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        for(auto it = urls.begin(); it != urls.end(); it++) {
            const QUrl& url = *it;
            QString localFile = url.toLocalFile();
            if(QFile::exists(localFile)) {
                de->setDropAction(Qt::MoveAction);
                de->setAccepted(true);
                return;
            }
        }
    }else{
        de->ignore();
    }
}

void QWoSftpWidget::handleLocalDropEvent(QDropEvent *de)
{
    const QMimeData *md = de->mimeData();
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        for(auto it = urls.begin(); it != urls.end(); it++) {
            const QUrl& url = *it;
            QString localFile = url.toLocalFile();
            QFileInfo fi(localFile);
            if(fi.exists()) {
                if(fi.isDir()) {
                    m_localModel->setPath(fi.absoluteFilePath());
                }else{
                    m_localModel->setPath(fi.path());
                }
                return;
            }
        }
    }else{
        de->ignore();
    }
}

void QWoSftpWidget::handleRemoteDragEnterEvent(QDropEvent *de)
{
    const QMimeData *md = de->mimeData();
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        for(auto it = urls.begin(); it != urls.end(); it++) {
            const QUrl& url = *it;
            QString localFile = url.toLocalFile();
            if(QFile::exists(localFile)) {
                de->setDropAction(Qt::MoveAction);
                de->setAccepted(true);
                return;
            }
        }
    }else{
        de->ignore();
    }
}

void QWoSftpWidget::handleRemoteDropEvent(QDropEvent *de)
{
    const QMimeData *md = de->mimeData();
    QStringList tasks;
    if(md->hasUrls()) {
        const QList<QUrl>& urls = md->urls();
        for(auto it = urls.begin(); it != urls.end(); it++) {
            const QUrl& url = *it;
            QString fileLocal = url.toLocalFile();
            tasks.append(fileLocal);
        }
    }
    if(!tasks.isEmpty()) {
        QMetaObject::invokeMethod(this, "runUploadTask", Qt::QueuedConnection, Q_ARG(QStringList, tasks));
    }
}

void QWoSftpWidget::handleView(const QString &fileSave, const QString &fileRemote)
{
    QFileInfo fi(fileSave);
    if(!fi.exists()) {
        QKxMessageBox::information(this, tr("File error"), tr("The temp file has been lost."));
        return;
    }

#if defined (Q_OS_MAC)
    QString cmd = QString("open -t \"%1\"").arg(fileSave);
    openEditor(cmd, fileSave, fileRemote, fi.lastModified());
#elif defined(Q_OS_LINUX)
    QStringList cmds = {"/usr/bin/open"};
    for(int i = 0; i < cmds.length(); i++) {
        QString program = cmds.at(i);
        if(!QFile::exists(program)) {
            continue;
        }
        QString cmd = QString("%1 \"%2\"").arg(program).arg(fileSave);
        openEditor(cmd, fileSave, fileRemote, fi.lastModified());
        return;
    }
#elif defined(Q_OS_WIN)
    QString cmd = QString("start \"%1\"").arg(fileSave);
    openEditor(cmd, fileSave, fileRemote, fi.lastModified());
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileSave));
#endif
}

void QWoSftpWidget::handleEdit(const QString &fileSave, const QString &fileRemote)
{
    QVariantMap dm = QWoSetting::value("sftpTool/textEditor").toMap();
    if(dm.isEmpty()) {
        QKxMessageBox::information(this, tr("Editor error"), tr("Please configure the text editing tool in the tool options dialog box first."));
        return;
    }

    QFileInfo fi(fileSave);
    if(!fi.exists()) {
        QKxMessageBox::information(this, tr("File error"), tr("The temp file has been lost."));
        return;
    }

    QString path = dm.value("path").toString();
    QString params = dm.value("arguments").toString();
    params.replace("{file}", fileSave);

    openEditor(path + " " + params, fileSave, fileRemote, fi.lastModified());
}

void QWoSftpWidget::handleEditCommit(const QString &fileSave, const QString &fileRemote, const qint64& dt)
{
    QFileInfo fi(fileSave);
    if(!fi.exists()) {
        return;
    }
    QDateTime lastModified = QDateTime::fromMSecsSinceEpoch(dt);
    if(fi.lastModified() == lastModified) {
        return;
    }
    int retval = QKxMessageBox::information(this, tr("Modify information"), tr("The content has been modified. Do you need to submit it to the server?"), QMessageBox::Yes|QMessageBox::No);
    if(retval == QMessageBox::Yes && m_sftp) {
        QVariantMap user;
        user.insert("command", "editFile");
        user.insert("local", fileSave);
        user.insert("remote", fileRemote);
        m_sftp->upload(fileSave, fileRemote, QWoSshFtp::TP_Override, user);
    }
}

QProcess* QWoSftpWidget::openEditor(const QString &cmd, const QString& fileSave, const QString& fileRemote, const QDateTime& lastModified)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcess *editor = new QProcess(this);
    QObject::connect(editor, SIGNAL(finished(int)), this, SLOT(onEditorDestroy()));
    QObject::connect(editor, SIGNAL(finished(int)), editor, SLOT(deleteLater()));
    QObject::connect(editor, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onEditorDestroy()));
    QObject::connect(editor, SIGNAL(errorOccurred(QProcess::ProcessError)), editor, SLOT(deleteLater()));
    editor->setProperty("fileSave", fileSave);
    editor->setProperty("fileRemote", fileRemote);
    editor->setProperty("lastModified", lastModified);
    editor->setProcessEnvironment(env);
    editor->start(cmd);
    m_editors.append(editor);
    return editor;
}

void QWoSftpWidget::reconnect()
{
    if(m_passInput) {
        m_passInput->deleteLater();
    }
    release();
    m_transfer->release();
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(dirOpen(QString,QVariantList,QVariantMap)), m_remoteModel, SLOT(onDirOpen(QString,QVariantList,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(passwordArrived(QString,QByteArray)), this, SLOT(onPasswordArrived(QString,QByteArray)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));

    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));

    m_sftp->start(m_target, m_gid);

    QString path = QWoSetting::value(QString("sftp/lastPathRemote:%1").arg(m_target), "").toString();
    QStringList paths;
    if(!path.isEmpty()) {
        paths.append(path);
    }
    paths.append("~");
    openDir(paths);
    m_loading->show();
}

/**
 * In the loop process, the model is often triggered to reload, resulting in a crash, so this function is added
 */
QList<FileInfo> QWoSftpWidget::remoteSelections()
{
    QItemSelectionModel *model = m_remote->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QList<FileInfo> lsfi;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
        if(fi.name == "." || fi.name == "..") {
            continue;
        }
        lsfi.append(fi);
    }
    return lsfi;
}

QList<QFileInfo> QWoSftpWidget::localSelections()
{
    QItemSelectionModel *model = m_local->selectionModel();
    QModelIndexList idxs = model->selectedRows();
    QList<QFileInfo> lsfi;
    for(int i = 0; i < idxs.length(); i++) {
        QModelIndex idx = idxs.at(i);
        QFileInfo fi = idx.data(ROLE_FILEINFO).value<QFileInfo>();
        if(fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }
        lsfi.append(fi);
    }
    return lsfi;
}

void QWoSftpWidget::runUploadTask(const QList<QFileInfo> &lsfi)
{
    QStringList same;
    int typeTransfer = QMessageBox::No;
    for(int i = 0; i < lsfi.length(); i++) {
        QFileInfo fi = lsfi.at(i);
        if(m_remoteModel->exist(fi.fileName())) {
            if(typeTransfer == QMessageBox::Yes ||
                    typeTransfer == QMessageBox::No){
                QMessageBox::StandardButtons buttonType = QMessageBox::Yes|QMessageBox::No;
                if(m_isUltimate && lsfi.length() > 1) {
                    buttonType |= QMessageBox::YesToAll|QMessageBox::NoToAll;
                }
                typeTransfer = QKxMessageBox::warning(this, tr("FileExist"),
                                                    tr("has the same name in the target path. override it?"),
                                                    buttonType);
                if(!m_isUltimate && typeTransfer != QMessageBox::Yes) {
                    return;
                }
            }
        }
        QString path = m_remoteModel->path();
        QString fileRemote = QDir::cleanPath(path + "/" + fi.fileName());
        bool isAppend = true;
        if(typeTransfer == QMessageBox::Yes || typeTransfer == QMessageBox::YesToAll) {
            isAppend = false;
        }
        QString fileLocal = fi.absoluteFilePath();
        if(!m_transfer->addTask(fileLocal, fileRemote, fi.isDir(), false, isAppend)) {
            same.append(fileLocal);
        }
    }
    if(!same.isEmpty()) {
        QString msg = same.join('\n');
        if(m_isUltimate) {
            QKxMessageBox::message(this, tr("Upload information"), tr("the follow files has exist:")+"\n"+msg, false);
        }else{
            QKxMessageBox::message(this, tr("Upload information"), tr("the follow files has exist or version restrictions:")+"\n"+msg, false);
        }
    }
}

void QWoSftpWidget::runUploadTask(const QStringList &lsf)
{
    if(QKxMessageBox::information(this, tr("Upload task"), tr("Do you want to upload this related file?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    QList<QFileInfo> files;
    for(int i = 0; i < lsf.length(); i++) {
        QFileInfo fi(lsf.at(i));
        files.append(fi);
    }
    runUploadTask(files);
}

void QWoSftpWidget::onRemoteContextMenuRequested(const QPoint& pos)
{
    QList<FileInfo> lsfi = remoteSelections();

    QMenu menu(this);
    if(lsfi.isEmpty()) {
        menu.addAction(tr("Select all"), this, SLOT(onRemoteMenuSelectAll()));
        menu.addAction(QIcon("../private/skins/black/back3.png"), tr("&Back"), this, SLOT(onRemoteMenuReturnTopDirectory()));
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("Refresh"), this, SLOT(onRemoteMenuReloadDirectory()));
        menu.addAction(QIcon("../private/skins/black/home.png"), tr("Home Directory"), this, SLOT(onRemoteMenuGoHomeDirectory()));
        menu.addAction(QIcon("../private/skins/black/mkdir.png"), tr("&Create Directory"), this, SLOT(onRemoteMenuCreateDirectory()));
        if(m_bAssist){
            menu.addAction(QIcon("../private/skins/black/upload2.png"), tr("Upload"), this, SLOT(onRemoteMenuUpload()));
        }
    }else{
        FileInfo fi = lsfi.at(0);
        menu.addAction(tr("Deselect all"), this, SLOT(onRemoteMenuDeselectAll()));
        menu.addAction(QIcon("../private/skins/black/back3.png"), tr("&Back"), this, SLOT(onRemoteMenuReturnTopDirectory()));
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("Refresh"), this, SLOT(onRemoteMenuReloadDirectory()));
        menu.addAction(QIcon("../private/skins/black/home.png"), tr("Home Directory"), this, SLOT(onRemoteMenuGoHomeDirectory()));
        menu.addAction(QIcon("../private/skins/black/mkdir.png"), tr("&Create Directory"), this, SLOT(onRemoteMenuCreateDirectory()));
        menu.addAction(QIcon("../private/skins/black/rmfile.png"), tr("&Delete selections"), this, SLOT(onRemoteMenuRemoveSelection()));
        menu.addAction(QIcon("../private/skins/black/permission.png"), tr("Modify the permission"), this, SLOT(onRemoteModifyItemPermission()));
        if(lsfi.length() == 1) {
            menu.addAction(tr("Rename"), this, SLOT(onRemoteMenuRename()));
            if(fi.isFile()) {
                menu.addAction(tr("Edit file content"), this, SLOT(onRemoteMenuEditFileContent()));
                menu.addAction(tr("View file content"), this, SLOT(onRemoteMenuViewFileContent()));
            }
        }
        menu.addAction(tr("Move to other directory"), this, SLOT(onRemoteMenuMoveToOtherDirectory()));
        if(fi.isDir()) {
            menu.addAction(QIcon("../private/skins/black/enter.png"), tr("Enter"), this, SLOT(onRemoteMenuEnterDirectory()));
            if(m_isUltimate) {
                menu.addAction(QIcon("../private/skins/black/download2.png"), tr("Download"), this, SLOT(onRemoteMenuDownload()));
            }
        }else if(fi.isFile()){
            menu.addAction(QIcon("../private/skins/black/download2.png"), tr("Download"), this, SLOT(onRemoteMenuDownload()));
        }else if(fi.isLink()) {
            menu.addAction(QIcon("../private/skins/black/enter3.png"), tr("Try Enter"), this, SLOT(onRemoteMenuTryEnterDirectory()));
        }
        if(m_bAssist){
            menu.addAction(QIcon("../private/skins/black/upload2.png"), tr("Upload"), this, SLOT(onRemoteMenuUpload()));
        }
    }
    menu.addAction(QIcon("../private/skins/black/multiplex.png"), tr("New Session Multiplex"), this, SLOT(onNewSessionMultiplex()));
    if(m_bAssist) {
        menu.addAction(tr("Close"), this, SLOT(onForceToCloseThisSession()));
    }
    menu.exec(QCursor::pos());
}

void QWoSftpWidget::onRemoteMenuSelectAll()
{
    m_remote->selectAll();
}

void QWoSftpWidget::onRemoteMenuDeselectAll()
{
    m_remote->clearSelection();
}

void QWoSftpWidget::onFinishArrived(int code)
{
    //qDebug() << "exitcode" << code;      
    if(m_mask && m_mask->isVisible()) {
        return;
    }
    if(m_transfer) {
        m_transfer->release();
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

void QWoSftpWidget::onErrorArrived(const QString &err, const QVariantMap& userData)
{
    QKxMessageBox::warning(this, tr("Error"), err);
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

void QWoSftpWidget::onLocalItemDoubleClicked(const QModelIndex &idx)
{
    QFileInfo fi = m_localModel->fileInfo(idx);
    if(fi.fileName() == "..") {
        QString path = m_localModel->path();
        if(path.isEmpty()) {
            return;
        }
        QFileInfo fi(path);
        if(fi.isRoot()) {
            m_localModel->setHome();
        }else{
            QString path=fi.absolutePath();
            m_localModel->setPath(path);
        }
        return;
    }
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_localModel->setPath(path);
    }else if(fi.isFile()) {
        if(1||QKxVer::instance()->isFullFeather() && QWoSetting::allowSftpToOpenLocalFile()){
            QDesktopServices::openUrl(fi.absoluteFilePath());
        }
    }
}

void QWoSftpWidget::onLocalContextMenuRequested(const QPoint &pos)
{
    QList<QFileInfo> lsfi = localSelections();
    QMenu menu(this);
    if(lsfi.isEmpty()) {
        menu.addAction(tr("Select all"), this, SLOT(onLocalMenuSelectAll()));
        menu.addAction(QIcon("../private/skins/black/back3.png"), tr("&Back"), this, SLOT(onLocalMenuReturnTopDirectory()));
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("Refresh"), this, SLOT(onLocalMenuReloadDirectory()));
        menu.addAction(QIcon("../private/skins/black/home.png"), tr("Home directory"), this, SLOT(onLocalMenuGoHomeDirectory()));
        menu.addAction(QIcon("../private/skins/black/mkdir.png"), tr("&Create directory"), this, SLOT(onLocalMenuCreateDirectory()));
    }else{
        QFileInfo fi = lsfi.at(0);

        menu.addAction(tr("Deselect all"), this, SLOT(onLocalMenuDeselectAll()));
        if(lsfi.size() == 1) {
            menu.addAction(tr("Rename"), this, SLOT(onLocalMenuFileRename()));
        }
        menu.addAction(QIcon("../private/skins/black/back3.png"), tr("&Back"), this, SLOT(onLocalMenuReturnTopDirectory()));
        menu.addAction(QIcon("../private/skins/black/reload.png"), tr("Refresh"), this, SLOT(onLocalMenuReloadDirectory()));
        menu.addAction(QIcon("../private/skins/black/home.png"), tr("Home directory"), this, SLOT(onLocalMenuGoHomeDirectory()));
        menu.addAction(QIcon("../private/skins/black/mkdir.png"), tr("&Create directory"), this, SLOT(onLocalMenuCreateDirectory()));
        menu.addAction(QIcon("../private/skins/black/rmfile.png"), tr("&Delete selections"), this, SLOT(onLocalMenuRemoveSelection()));
        if(fi.isDir()) {
            menu.addAction(QIcon("../private/skins/black/enter.png"), tr("Enter"), this, SLOT(onLocalMenuEnterDirectory()));
        }else if(fi.isSymLink()) {
            menu.addAction(QIcon("../private/skins/black/enter3.png"), tr("Try enter"), this, SLOT(onLocalMenuTryEnterDirectory()));
        }
        menu.addAction(QIcon("../private/skins/black/upload2.png"), tr("Upload"), this, SLOT(onLocalMenuUpload()));

    }
    QPoint pt = QCursor::pos();
    menu.exec(pt);
}

void QWoSftpWidget::onLocalMenuDeselectAll()
{
    m_local->clearSelection();
}

void QWoSftpWidget::onLocalMenuFileRename()
{
    QList<QFileInfo> lsfi = localSelections();
    if(lsfi.isEmpty()) {
        return;
    }
    QFileInfo fi = lsfi.at(0);
    QString path = fi.absolutePath();
    QWoSftpRenameDialog dlg(path, fi.fileName(), this);
    if(dlg.exec() == QDialog::Accepted+1) {
        QString pathOld = path + "/" + fi.fileName();
        QString pathNew = dlg.pathResult();
        int idx = pathNew.lastIndexOf('/');
        if(idx > 0) {
            QDir d;
            if(!d.mkpath(pathNew.left(idx))) {
                QKxMessageBox::information(this, tr("Failure"), tr("Failed to rename file for no permission to make path."));
                return;
            }
        }
        if(QFile::rename(pathOld, pathNew)) {
            onLocalMenuReloadDirectory();
        }
    }
}

void QWoSftpWidget::onLocalMenuSelectAll()
{
    m_local->selectAll();
}

void QWoSftpWidget::onLocalMenuReturnTopDirectory()
{
    QString path = m_localModel->path();
    if(path.isEmpty()) {
        m_localModel->setHome();
        return;
    }
    QFileInfo fi(path);
    if(fi.isRoot()) {
        m_localModel->setHome();
    }else{
        QString path = fi.absolutePath();
        m_localModel->setPath(path);
    }
}

void QWoSftpWidget::onLocalMenuGoHomeDirectory()
{
    m_localModel->setHome();
}

void QWoSftpWidget::onLocalMenuReloadDirectory()
{
    QString path = ui->localPath->text();
    m_localModel->setPath(path);
}

void QWoSftpWidget::onLocalMenuCreateDirectory()
{
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
        QKxMessageBox::message(this, tr("information"), tr("the new directory name should be empty!"));
        return;
    }
    QString path = ui->localPath->text();
    if(path.isEmpty()) {
        return;
    }
    QDir d(path);
    if(d.mkdir(name)) {
        m_localModel->reload();
    }
}

void QWoSftpWidget::onLocalMenuRemoveSelection()
{
    QList<QFileInfo> lsfi = localSelections();
    if(lsfi.isEmpty()) {
        QKxMessageBox::warning(this, "delete", "no selection?", QMessageBox::Ok|QMessageBox::No);
        return;
    }
    if(QKxMessageBox::information(this, tr("Remove file"), tr("After deleting the file, it will not be recoverable. Please confirm whether to continue deleting the file."), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    for(int i = 0; i < lsfi.length(); i++) {
        QFileInfo fi = lsfi.at(i);
        if(fi.isDir()) {
            QWoUtils::removeDirectory(fi.absoluteFilePath());
        }else {
            QFile::remove(fi.absoluteFilePath());
        }
    }
    m_localModel->reload();
}

void QWoSftpWidget::onLocalMenuEnterDirectory()
{
    QModelIndex idx = m_local->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    QFileInfo fi = m_localModel->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_localModel->setPath(path);
    }
}

void QWoSftpWidget::onLocalMenuTryEnterDirectory()
{
    onLocalMenuEnterDirectory();
}

void QWoSftpWidget::onLocalMenuUpload()
{
    if(!canTransfer()) {
        return;
    }
    QList<QFileInfo> lsfi = localSelections();
    runUploadTask(lsfi);
}

void QWoSftpWidget::onLocalResetModel()
{
    for(int i = 0; i < 3;i++) {
        m_local->resizeColumnToContents(i);
    }
}

void QWoSftpWidget::onLocalPathReturnPressed()
{
    QString path = ui->localPath->text();
    if(path.startsWith('~')) {
        path.replace(0, 1, QDir::homePath());
    }
    QString pathHit = m_localModel->path();
    if(path == pathHit) {
        return;
    }
    if(path.isEmpty()) {
        m_localModel->setHome();
    }else{
        QFileInfo fi(path);
        if(!fi.exists()) {
            QKxMessageBox::message(this, tr("Input error"), tr("The directory entered does not exist."), false);
            return;
        }
        if(fi.isDir()) {
            QString path = fi.absoluteFilePath();
            m_localModel->setPath(path);
        }else{
            QString path = fi.path();
            m_localModel->setPath(path);
        }
    }
}

void QWoSftpWidget::onRemoteItemDoubleClicked(const QModelIndex &idx)
{
    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    qDebug() << "server:" << fi.name;
    if(fi.isDir()) {
        QString path = m_remoteModel->path() + "/" + fi.name;
        openDir(path);
    }
}

void QWoSftpWidget::onRemoteMenuReturnTopDirectory()
{
    QString path = QDir::cleanPath(m_remoteModel->path() + "/..");
    openDir(path);
}

void QWoSftpWidget::onRemoteMenuGoHomeDirectory()
{
    openDir("~");
}

void QWoSftpWidget::onRemoteMenuReloadDirectory()
{
    QString path = m_remoteModel->path();
    openDir(path);
}

void QWoSftpWidget::onRemoteMenuCreateDirectory()
{
    QWoSftpNameDialog dlg("", this);
    dlg.exec();
    QString fileName = dlg.result();
    if(fileName.isEmpty()) {
        return;
    }
    bool isPrivate = dlg.isPrivate();
    QString path = m_remoteModel->path();
    path = QDir::cleanPath(path + "/" + fileName);
    mkDir(path, isPrivate ? 0x1C0 : 0x1FF);
}

void QWoSftpWidget::onRemoteMenuRemoveSelection()
{
    auto lsfi = remoteSelections();
    if(QKxMessageBox::information(this, tr("Remove file"), tr("After deleting the file, it will not be recoverable. Please confirm whether to continue deleting the file."), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
        return;
    }
    for(auto it = lsfi.begin(); it != lsfi.end(); it++){
        const FileInfo& fi = *it;
        if(fi.name == "." || fi.name == "..") {
            continue;
        }
        QString path = m_remoteModel->path();
        path = QDir::cleanPath(path+"/"+fi.name);
        if(fi.isDir()){
            rmDir(path);
        }else{
            unlink(path);
        }
    }
}

void QWoSftpWidget::onRemoteModifyItemPermission()
{
    auto lsfi = remoteSelections();
    if(lsfi.isEmpty()) {
        return;
    }
    const FileInfo& fi = lsfi.first();
    QString path = m_remoteModel->path();
    QString filePath = QDir::cleanPath(path+"/"+fi.name);
    QWoSftpPermissionDialog dlg(filePath, fi.permission, this);
    if(dlg.exec() == QDialog::Accepted+1) {
        bool subdirs;
        int perm = dlg.permssionResult(&subdirs);
        if(lsfi.length() > 1 && QKxMessageBox::information(this, tr("Permission information"), tr("Have other files also been modified with the same access permissions?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
            for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
                const FileInfo& fi = *it;
                QString filePath = QDir::cleanPath(path+"/"+fi.name);
                QVariantMap user;
                user.insert("command", "chmod");
                user.insert("path", filePath);
                user.insert("permission", perm);
                m_sftp->chmod(filePath, perm, subdirs && fi.isDir(), user);
            }
        }else{
            QString filePath = QDir::cleanPath(path+"/"+fi.name);
            QVariantMap user;
            user.insert("command", "chmod");
            user.insert("path", filePath);
            user.insert("permission", perm);
            m_sftp->chmod(filePath, perm, subdirs && fi.isDir(), user);
        }
    }
}

void QWoSftpWidget::onRemoteMenuEnterDirectory()
{
    QModelIndex idx = m_remote->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    FileInfo fi = idx.data(ROLE_FILEINFO).value<FileInfo>();
    QString path = m_remoteModel->path();
    openDir(path + "/" + fi.name);
}

void QWoSftpWidget::onRemoteMenuTryEnterDirectory()
{
    onRemoteMenuEnterDirectory();
}

void QWoSftpWidget::onRemoteMenuDownload()
{
    if(!canTransfer()) {
        return;
    }
    QList<FileInfo> lsfi = remoteSelections();
    if(lsfi.isEmpty()) {
        return;
    }

    QString path = m_remoteModel->path();
    QString pathSave;
    if(m_bAssist){
        QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
        pathSave = QFileDialog::getExistingDirectory(this, tr("Save directory"), pathLast);
        if(pathSave.isEmpty()) {
            return;
        }
    }else{
        pathSave = m_localModel->path();
        if(pathSave.isEmpty()) {
            QKxMessageBox::message(this, tr("Parameter error"), tr("Please select directory to save it."), false);
            return;
        }
    }
    QWoSetting::setValue("zmodem/lastPath", pathSave);

    QStringList same;
    int typeTransfer = QMessageBox::No;
    for(int i = 0; i < lsfi.length(); i++) {
        FileInfo fi = lsfi.at(i);
        if(fi.name == "." || fi.name == "..") {
            continue;
        }
        QString path = m_remoteModel->path();
        QString fileLocal = pathSave + "/" + fi.name;
        QString fileRemote = QDir::cleanPath(path + "/" + fi.name);
        if(QFile::exists(fileLocal)) {
            if(typeTransfer == QMessageBox::Yes ||
                    typeTransfer == QMessageBox::No){
                QMessageBox::StandardButtons buttonType = QMessageBox::Yes|QMessageBox::No;
                if(m_isUltimate && lsfi.length() > 1) {
                    buttonType |= QMessageBox::YesToAll|QMessageBox::NoToAll;
                }
                typeTransfer = QKxMessageBox::warning(this, tr("FileExist"),
                                                    tr("has the same name in the target path. override it?"),
                                                    buttonType);
                if(!m_isUltimate && typeTransfer != QMessageBox::Yes) {
                    return;
                }
            }
        }
        bool isAppend = true;
        if(typeTransfer == QMessageBox::Yes || typeTransfer == QMessageBox::YesToAll) {
            isAppend = false;
        }
        if(!m_transfer->addTask(fileLocal, fileRemote, fi.isDir(), true, isAppend)) {
            same.append(fileRemote);
        }
    }
    if(!same.isEmpty()) {
        QString msg = same.join('\n');
        if(m_isUltimate) {
            QKxMessageBox::message(this, tr("Download information"), tr("the follow files has exist:")+"\n"+msg, false);
        }else{
            QKxMessageBox::message(this, tr("Download information"), tr("the follow files has exist or version restrictions:")+"\n"+msg, false);
        }
    }
}

void QWoSftpWidget::onRemoteMenuUpload()
{
    if(!canTransfer()) {
        return;
    }
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
    QStringList files;
    if(m_isUltimate) {
        files = QFileDialog::getOpenFileNames(this, tr("Select files"), pathLast);
        if(files.isEmpty()) {
            return;
        }
    }else{
        QString fileLocal = QFileDialog::getOpenFileName(this, tr("Select File"), pathLast);
        if(fileLocal.isEmpty()) {
            return;
        }
        files.append(fileLocal);
    }
    for(int i = 0; i < files.length(); i++) {
        QString fileLocal = files.at(i);
        QString name = fileLocal;
        int idx = fileLocal.lastIndexOf("/");
        if(idx > 0) {
            name = fileLocal.mid(idx);
        }
        QString path = m_remoteModel->path();
        QString fileRemote = QDir::cleanPath(path + name);
        QFileInfo fi(fileLocal);
        m_transfer->addTask(fileLocal, fileRemote, fi.isDir(), false, true);
    }
}

void QWoSftpWidget::onNewSessionMultiplex()
{
    QWoShower *shower = QWoMainWindow::instance()->shower();
    shower->openSftp(m_target, m_gid);
}

void QWoSftpWidget::onRemoteResetModel()
{
    for(int i = 0; i < 5;i++) {
        m_remote->resizeColumnToContents(i);
    }
}

void QWoSftpWidget::onRemotePathReturnPressed()
{
    QString path = ui->remotePath->text();
    QString pathHit = m_remoteModel->path();
    if(path == pathHit) {
        return;
    }
    if(path.isEmpty()) {
        openDir("/");
    }else if(m_sftp) {
        QVariantMap dm;
        dm.insert("path", path);
        dm.insert("customEnter", true);
        m_sftp->fileInfo(path, dm);
    }
}

void QWoSftpWidget::onRemoteMenuRename()
{
    QList<FileInfo> lsfi = remoteSelections();
    if(lsfi.isEmpty()) {
        return;
    }
    if(lsfi.length() > 1) {
        return;
    }
    QString path = m_remoteModel->path();
    const FileInfo& fi = lsfi.takeFirst();
    QWoSftpRenameDialog dlg(path, fi.name, this);
    if(dlg.exec() == QDialog::Accepted+1) {
        QString pathOld = path + "/" + fi.name;
        QString pathNew = dlg.pathResult();
        QVariantMap user;
        user.insert("command", "rename");
        user.insert("pathOld", pathOld);
        user.insert("pathNew", pathNew);
        m_sftp->rename(pathOld, pathNew, user);
    }
}

void QWoSftpWidget::onRemoteMenuMoveToOtherDirectory()
{
    QList<FileInfo> lsfi = remoteSelections();
    if(lsfi.isEmpty()) {
        return;
    }
    QString path = m_remoteModel->path();
    bool ok = false;
    QString pathSave = QInputDialog::getText(this, tr("Move file"), tr("Input a existing directory to save the selected file."), QLineEdit::Normal, path, &ok, Qt::Dialog|Qt::WindowCloseButtonHint);
    if(pathSave.isEmpty() || pathSave == path || ok == false) {
        return;
    }
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        const FileInfo& fi = *it;
        QString pathOld = path + "/" + fi.name;
        QString pathNew = pathSave + "/" + fi.name;
        QVariantMap user;
        user.insert("command", "moveFile");
        user.insert("pathOld", pathOld);
        user.insert("pathNew", pathNew);
        m_sftp->rename(pathOld, pathNew, user);
    }
}

void QWoSftpWidget::onRemoteMenuEditFileContent()
{
    QList<FileInfo> lsfi = remoteSelections();
    if(lsfi.length() != 1) {
        return;
    }

    QString path = m_remoteModel->path();
    const FileInfo& fi = lsfi.first();
    if(fi.isDir()) {
        QKxMessageBox::message(this, tr("File information"), tr("Directory is not supported to edit."));
        return;
    }
    if(fi.size > (1024 * 1024 * 5)) {
        QKxMessageBox::message(this, tr("File information"), tr("The file size exceeding 5M bytes is not supported."));
        return;
    }
    if(fi.size > (500*1024)) {
        if(QKxMessageBox::information(this, tr("File information"), tr("The target file exceeds 500K bytes. Do you want to continue editing?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }    
    QString filePath = QDir::cleanPath(path + "/" + fi.name);
    QString fileSave = QWoSetting::cachePath() + "/" + fi.name;
    QVariantMap user;
    user.insert("command", "editFile");
    user.insert("local", fileSave);
    user.insert("remote", filePath);
    m_sftp->download(filePath, fileSave, QWoSshFtp::TP_Override, user);
}

void QWoSftpWidget::onRemoteMenuViewFileContent()
{
    QList<FileInfo> lsfi = remoteSelections();
    if(lsfi.length() != 1) {
        return;
    }

    QString path = m_remoteModel->path();
    const FileInfo& fi = lsfi.first();
    if(fi.isDir()) {
        QKxMessageBox::message(this, tr("File information"), tr("Directory is not supported to view."));
        return;
    }
    if(fi.size > (1024 * 1024 * 5)) {
        QKxMessageBox::message(this, tr("File information"), tr("The file size exceeding 5M bytes is not supported."));
        return;
    }
    if(fi.size > (500*1024)) {
        if(QKxMessageBox::information(this, tr("File information"), tr("The target file exceeds 500K bytes. Do you want to continue view?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }
    QString filePath = QDir::cleanPath(path + "/" + fi.name);
    QString fileSave = QWoSetting::cachePath() + "/" + fi.name;
    QVariantMap user;
    user.insert("command", "viewFile");
    user.insert("local", fileSave);
    user.insert("remote", filePath);
    m_sftp->download(filePath, fileSave, QWoSshFtp::TP_Override, user);
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

void QWoSftpWidget::tryToSyncPath(const QString &_path)
{
    QVariant v = ui->btnFollow->property(SYNC_FOLLOW_FLAG);
    if(m_sftp && v.toBool()) {
        QString tmp = ui->remotePath->text();
        QString path = QDir::cleanPath(_path);
        if(tmp != path) {
            ui->remotePath->setText(path);
            QVariantMap dm;
            dm.insert("path", path);
            m_sftp->openDir(path, dm);
        }
    }
}

void QWoSftpWidget::onRemoteHomeButtonClicked()
{
    onRemoteMenuGoHomeDirectory();
}

void QWoSftpWidget::onRemoteBackButtonClicked()
{
    onRemoteMenuReturnTopDirectory();
}

void QWoSftpWidget::onRemoteForwardButtonClicked()
{
    const QModelIndex& idx = m_remote->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::message(this, tr("SFTP"), tr("No items are currently selected."), false);
        return;
    }
    onRemoteItemDoubleClicked(idx);
}

void QWoSftpWidget::onRemoteReloadButtonClicked()
{
    onRemoteMenuReloadDirectory();
}

void QWoSftpWidget::onRemoteFollowButtonClicked()
{
    QVariant v = ui->btnFollow->property(SYNC_FOLLOW_FLAG);
    if(v.toBool()) {
        ui->btnFollow->setProperty(SYNC_FOLLOW_FLAG, false);
        ui->btnFollow->setIcon(QIcon("../private/skins/black/select2.png"));
        ui->btnFollow->setToolTip(tr("The directory is out of sync with the terminal."));
    }else{
        ui->btnFollow->setProperty(SYNC_FOLLOW_FLAG, true);
        ui->btnFollow->setIcon(QIcon("../private/skins/black/select4.png"));
        ui->btnFollow->setToolTip(tr("Keep the directory synchronized with the terminal."));
    }
}

void QWoSftpWidget::onRemoteTransferButtonClicked()
{
    m_transfer->setVisible(!m_transfer->isVisible());
}

void QWoSftpWidget::onRemotePathChanged(const QString &path)
{
    ui->remotePath->setText(path);
    QWoSetting::setValue(QString("sftp/lastPathRemote:%1").arg(m_target), path);
}

void QWoSftpWidget::onRemoteDropArrived(const QList<QUrl> &urls)
{
    QStringList tasks;
    for(auto it = urls.begin(); it != urls.end(); it++) {
        const QUrl& url = *it;
        QString fileLocal = url.toLocalFile();
        tasks.append(fileLocal);
    }
    if(!tasks.isEmpty()) {
        QMetaObject::invokeMethod(this, "runUploadTask", Qt::QueuedConnection, Q_ARG(QStringList, tasks));
    }
}

void QWoSftpWidget::onLocalHomeButtonClicked()
{
    m_localModel->setHome();
}

void QWoSftpWidget::onLocalBackButtonClicked()
{
    QString path = m_localModel->path();
    if(path.isEmpty()) {
        return;
    }
    QFileInfo fi(path);
    if(fi.isRoot()) {
        m_localModel->setHome();
    }else {
        QString path = fi.absolutePath();
        m_localModel->setPath(path);
    }
}

void QWoSftpWidget::onLocalForwardButtonClicked()
{
    const QModelIndex& idx = m_local->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::message(this, tr("SFTP"), tr("No items are currently selected."), false);
        return;
    }
    QFileInfo fi = m_localModel->fileInfo(idx);
    if(fi.isDir()) {
        QString path = fi.absoluteFilePath();
        m_localModel->setPath(path);
    }
}

void QWoSftpWidget::onLocalReloadButtonClicked()
{
    m_localModel->reload();
}

void QWoSftpWidget::onLocalBrowserButtonClicked()
{
    QString path = ui->localPath->text();
    QString url = "file:///"+path;
    QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void QWoSftpWidget::onLocalPathChanged(const QString &path)
{
    ui->localPath->setText(path);
    QWoSetting::setValue("sftp/lastPathLocal", path);
}

void QWoSftpWidget::onLocalDropArrived(const QList<QUrl> &urls)
{
    for(auto it = urls.begin(); it != urls.end(); it++) {
        const QUrl& url = *it;
        QString localFile = url.toLocalFile();
        QFileInfo fi(localFile);
        if(fi.exists()) {
            if(fi.isDir()) {
                m_localModel->setPath(fi.absoluteFilePath());
            }else{
                m_localModel->setPath(fi.path());
            }
            return;
        }
    }
}

void QWoSftpWidget::onEditorDestroy()
{
    QProcess *editor = qobject_cast<QProcess*>(sender());
    if(m_editors.removeAll(editor) > 0) {
        QString fileSave = editor->property("fileSave").toString();
        QString fileRemote = editor->property("fileRemote").toString();
        QDateTime dt = editor->property("lastModified").toDateTime();
        QMetaObject::invokeMethod(this, "handleEditCommit", Qt::QueuedConnection, Q_ARG(QString, fileSave), Q_ARG(QString, fileRemote), Q_ARG(qint64, dt.toMSecsSinceEpoch()));
    }
}

void QWoSftpWidget::onTransferCommandStart(int type, const QVariantMap& userData)
{
    if(!m_transfer->isVisible()) {
        QTimer::singleShot(0, this, SLOT(onTransferAdjustSize()));
    }
    m_transfer->show();
    m_transfer->raise();
}

/*
#define MT_FTP_OPENDIR      (10)
#define MT_FTP_MKDIR        (11)
#define MT_FTP_RMDIR        (12)
#define MT_FTP_UNLINK       (13)
#define MT_FTP_DOWNLOAD     (14)
#define MT_FTP_UPLOAD       (15)
#define MT_FTP_UPLOADNEXT   (16)
#define MT_FTP_ABORT        (17)
#define MT_FTP_LISTFILE     (18)
#define MT_FTP_LISTFILENEXT (19)
*/

void QWoSftpWidget::onTransferCommandFinish(int type, const QVariantMap& userData)
{
    if(type == 14) {
        // MT_FTP_DOWNLOAD
        if(m_localModel) {
            m_localModel->reload();
        }
    }else if(type == 11 || type == 12 || type == 13 || type == 15){
        // MT_FTP_UPLOAD
        onRemoteMenuReloadDirectory();
    }
}

void QWoSftpWidget::onTransferAdjustSize()
{
    m_transfer->adjustSize();
}

void QWoSftpWidget::onCommandStart(int t, const QVariantMap& userData)
{
    m_loading->show();
}

/*
 * qwossh.h
#define MT_FTP_OPENDIR      (10)
#define MT_FTP_MKDIR        (11)
#define MT_FTP_RMDIR        (12)
#define MT_FTP_UNLINK       (13)
*/
void QWoSftpWidget::onCommandFinish(int t, const QVariantMap& userData)
{
    m_loading->hide();
    QString reason = userData.value("reason").toString();
    if(t == MT_FTP_CHMOD) {
        onRemoteMenuReloadDirectory();
    }else if(t == MT_FTP_RENAME) {
        onRemoteMenuReloadDirectory();
    }else if(t == MT_FTP_DOWNLOAD) {
        QString command = userData.value("command").toString();
        if(command == "editFile" && reason == "ok") {
            QString fileSave = userData.value("local").toString();
            QString fileRemote = userData.value("remote").toString();
            QMetaObject::invokeMethod(this, "handleEdit", Qt::QueuedConnection, Q_ARG(QString,fileSave), Q_ARG(QString,fileRemote));
        }else if(command == "viewFile" && reason == "ok") {
            QString fileSave = userData.value("local").toString();
            QString fileRemote = userData.value("remote").toString();
            QMetaObject::invokeMethod(this, "handleView", Qt::QueuedConnection, Q_ARG(QString,fileSave), Q_ARG(QString,fileRemote));
        }
    }else if(t == MT_FTP_UPLOAD) {
        QString command = userData.value("command").toString();
        if(command == "editFile" && reason == "ok") {
            QString fileSave = userData.value("local").toString();
            QString fileRemote = userData.value("remote").toString();
            QKxMessageBox::message(this, tr("File information"), tr("The modified file[%1] has been successfully submitted.").arg(fileRemote));
        }
    }else if(t == MT_FTP_FILE_INFO) {
        bool customEnter = userData.value("customEnter").toBool();
        if(customEnter) {            
            QVariantMap dm = userData.value("fileInfo").toMap();
            if(dm.isEmpty()) {
                static bool showBox = false;
                if(showBox) {
                    return;
                }
                showBox = true;
                QString path = userData.value("path").toString();
                QKxMessageBox::message(this, tr("File information"), tr("The remote path[%1] does not exist or does not have permission to access it?").arg(path));
                showBox = false;
                return;
            }
            if(!m_sftp) {
                return;
            }
            QString path = dm.value("absPath").toString();
            QString name = dm.value("name").toString();
            QString type = dm.value("type").toString();
            if(type.isEmpty() || type.at(0) != "d") {
                m_sftp->openDir(path);
            }else{
                m_sftp->openDir(path + "/" + name);
            }
            return;
        }
    }
    if(reason == "fatal") {
        release();
    }else if(reason == "error") {
        QString err = userData.value("errorString").toString();
        if(!err.isEmpty()) {
            if(t == MT_FTP_CHMOD) {
                err += "\r\n";
                err += tr("Chmod");
                err += "\r\n";
                QString path = userData.value("path").toString();
                err += path;
            }else if(t == MT_FTP_RENAME){
                err += "\r\n";
                err += tr("Rename");
                err += "\r\n";
                QString pathOld = userData.value("pathOld").toString();
                QString pathNew = userData.value("pathNew").toString();
                err += "\r\n";
                err += pathOld;
                err += "\r\n";
                err += pathNew;
            }
            QKxMessageBox::message(this, tr("Error"), err);
        }
    }
    if(t == 11 || t == 12 || t == 13) {
        onRemoteMenuReloadDirectory();
    }
}
