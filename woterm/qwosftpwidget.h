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

#include <QWidget>
#include <QPointer>

namespace Ui {
class QWoSftpWidget;
}

class QWoSshFtp;
class QWoSftpListModel;
class QWoTermMask;
class QWoPasswordInput;
class QWoSftpDownMgrModel;
class QFileDialog;
class QWoTreeView;
class QSortFilterProxyModel;
class QWoSftpTransferWidget;
class QWoLoadingWidget;

class QWoSftpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QWoSftpWidget(const QString& target, int gid, QWidget *parent = 0);
    ~QWoSftpWidget();

private:
    void openHome();
    void openDir(const QString& path);
    void init(QObject *obj, const QString& type);
    void mkDir(const QString& path, const QString& name, int mode);
    void rmDir(const QString& path, const QString& name);
    void unlink(const QString& path, const QString& name);
    int textWidth(const QString& txt, const QFont& ft);
    void download(const QString& remote, const QString& local);
    void upload(const QString& local, const QString& remote);
protected:
    // for close safely.
    virtual QList<QString> collectUnsafeCloseMessage();
protected slots:
    void onTransferAbort();
    void onCommandStart(int type);
    void onCommandFinish(int type);
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onProgress(int type, int v);
    void onFinishArrived(int code);
    void onErrorArrived(const QByteArray& buf);
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onPasswordInputResult(const QString& pass, bool isSave);
    void onSessionReconnect();
    void onForceToCloseThisSession();
    void onListItemDoubleClicked(const QModelIndex& item);
    void onListCurrentItemChanged(const QModelIndex& item);
    void onListReturnKeyPressed();
    void onMenuReturnTopDirectory();
    void onMenuGoHomeDirectory();
    void onMenuReloadDirectory();
    void onMenuCreateDirectory();
    void onMenuRemoveDirectory();
    void onMenuRemoveFile();
    void onMenuEnterDirectory();
    void onMenuTryEnterDirectory();
    void onMenuDownload();
    void onMenuUpload();
    void onNewSessionMultiplex();
    void onResetModel();
    void onAdjustPosition();
protected:
    void showPasswordInput(const QString&title, const QString& prompt, bool echo);
    void resizeEvent(QResizeEvent *ev);
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    Q_INVOKABLE void reconnect();
private:
    bool handleListContextMenu(QContextMenuEvent* ev);

private:
    friend class QWoSftpWidgetImpl;
    Ui::QWoSftpWidget *ui;
    int m_gid;

    QPointer<QWoSftpListModel> m_model;
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QPointer<QWoSshFtp> m_sftp;
    QPointer<QWoPasswordInput> m_passInput;
    QPointer<QWoTermMask> m_mask;
    QPointer<QWoLoadingWidget> m_loading;
    QPointer<QWoTreeView> m_list;
    QPointer<QWoSftpTransferWidget> m_transfer;
    QString m_target;
    bool m_savePassword;
    bool m_bexit;
};

#endif // QWOSFTPWIDGET_H
