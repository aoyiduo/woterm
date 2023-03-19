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

#ifndef QWOSFTPWIDGET_H
#define QWOSFTPWIDGET_H

#include "qwoglobal.h"

#include <QWidget>
#include <QPointer>
#include <QList>
#include <QFileInfo>

namespace Ui {
class QWoSftpWidget;
}

class QWoSshFtp;
class QWoSftpLocalModel;
class QWoSftpRemoteModel;
class QWoTermMask;
class QWoPasswordInput;
class QFileDialog;
class QWoTreeView;
class QSortFilterProxyModel;
class QWoSftpTransferWidget;
class QWoLoadingWidget;
class QSplitter;
class QTreeView;

class QWoSftpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSftpWidget(const QString& target, int gid, bool assist, QWidget *parent = 0);
    ~QWoSftpWidget();

    void tryToSyncPath(const QString& path);
private:
    void openHome();
    void openDir(const QStringList& paths);
    void openDir(const QString& path);
    void init(QObject *obj, const QString& type);
    void mkDir(const QString& path, int mode);
    void rmDir(const QString& path);
    void unlink(const QString& path);
    int textWidth(const QString& txt, const QFont& ft);
    bool canTransfer();
    void release();
protected:
    // for close safely.
    virtual QList<QString> collectUnsafeCloseMessage();
protected slots:
    void onTransferAbort();
    void onTransferCommandStart(int type, const QVariantMap& userData);
    void onTransferCommandFinish(int type, const QVariantMap& userData);
    void onTransferAdjustSize();

    /*dir or file.*/
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onFinishArrived(int code);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onPasswordInputResult(const QString& pass, bool isSave);
    void onSessionReconnect();
    void onForceToCloseThisSession();
    void onLocalItemDoubleClicked(const QModelIndex& item);
    void onLocalContextMenuRequested(const QPoint &pos);
    void onLocalMenuDeselectAll();
    void onLocalMenuSelectAll();
    void onLocalMenuReturnTopDirectory();
    void onLocalMenuGoHomeDirectory();
    void onLocalMenuReloadDirectory();
    void onLocalMenuCreateDirectory();
    void onLocalMenuRemoveSelection();
    void onLocalMenuEnterDirectory();
    void onLocalMenuTryEnterDirectory();
    void onLocalMenuUpload();
    void onLocalResetModel();    
    void onLocalPathReturnPressed();

    void onRemoteItemDoubleClicked(const QModelIndex& item);
    void onRemoteContextMenuRequested(const QPoint& pos);
    void onRemoteMenuSelectAll();
    void onRemoteMenuDeselectAll();
    void onRemoteMenuReturnTopDirectory();
    void onRemoteMenuGoHomeDirectory();
    void onRemoteMenuReloadDirectory();
    void onRemoteMenuCreateDirectory();
    void onRemoteMenuRemoveSelection();
    void onRemoteMenuEnterDirectory();
    void onRemoteMenuTryEnterDirectory();
    void onRemoteMenuDownload();
    void onRemoteMenuUpload();
    void onRemoteResetModel();
    void onRemotePathReturnPressed();

    void onNewSessionMultiplex();
    void onAdjustPosition();

    /* remote */
    void onRemoteHomeButtonClicked();
    void onRemoteBackButtonClicked();
    void onRemoteForwardButtonClicked();
    void onRemoteReloadButtonClicked();
    void onRemoteFollowButtonClicked();
    void onRemoteTransferButtonClicked();
    void onRemotePathChanged(const QString& path);
    /* local */
    void onLocalHomeButtonClicked();
    void onLocalBackButtonClicked();
    void onLocalForwardButtonClicked();
    void onLocalReloadButtonClicked();
    void onLocalBrowserButtonClicked();
    void onLocalPathChanged(const QString& path);

protected:
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    void resizeEvent(QResizeEvent *ev);
    bool eventFilter(QObject *obj, QEvent *ev);
    void handleLocalDragEnterEvent(QDropEvent *de);
    void handleLocalDropEvent(QDropEvent *de);
    void handleRemoteDragEnterEvent(QDropEvent *de);
    void handleRemoteDropEvent(QDropEvent *de);

private:
    Q_INVOKABLE void reconnect();
    QList<FileInfo> remoteSelections();
    QList<QFileInfo> localSelections();
    void runUploadTask(const QList<QFileInfo>& lsf);
    Q_INVOKABLE void runUploadTask(const QStringList& lsfi);
private:
    friend class QWoSftpWidgetImpl;
    Ui::QWoSftpWidget *ui;
    int m_gid;
    bool m_isUltimate;

    QPointer<QWoSftpRemoteModel> m_remoteModel;
    QPointer<QWoSftpLocalModel> m_localModel;
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QPointer<QWoSshFtp> m_sftp;
    QPointer<QWoPasswordInput> m_passInput;
    QPointer<QWoTermMask> m_mask;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QTreeView> m_local, m_remote;
    QPointer<QWoSftpTransferWidget> m_transfer;
    QString m_target;
    bool m_savePassword;
    bool m_bexit;
    bool m_bAssist;
};

#endif // QWOSFTPWIDGET_H
