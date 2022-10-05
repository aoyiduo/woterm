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

#ifndef QKXFTPTRANSFERWINDOW_H
#define QKXFTPTRANSFERWINDOW_H

#include "qkxftp_share.h"

#include <QWidget>
#include <QPointer>
#include <QStyledItemDelegate>


class QTableView;
class QSplitter;
class QTreeView;
class QKxFtpLocalModel;
class QKxFtpRemoteModel;
class QKxFtpTransferModel;
class QKxFtpRemoteFileModel;
class QTimer;
class QKxFtpRequest;
class QLineEdit;
class QPushButton;

class QKxProgress : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QKxProgress(QWidget *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KXFTP_EXPORT QKxFtpTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QKxFtpTransferWidget(const QString& path, const QString& name, QWidget *parent = 0);
    ~QKxFtpTransferWidget();

    void setRequest(QKxFtpRequest *ftp);
signals:
    void reload();
    void closeArrived();
private slots:
    // local
    void onLocalPathReset();
    void onLocalHomeButtonClicked();
    void onLocalBackButtonClicked();
    void onLocalForwardButtonClicked();
    void onLocalRefreshButtonClicked();
    void onLocalBrowserButtonClicked();
    void onLocalItemClicked(const QModelIndex& idx);
    void onLocalItemDbClicked(const QModelIndex& idx);
    void onLocalContextMenuRequested(const QPoint &pos);
    void onLocalCreateDirectory();
    void onLocalFileRefresh();
    void onLocalBrowserDirectory();
    void onLocalFileUpload();
    void onBrowserLocalDirectory();
    // remote
    void onRemotePathReset();
    void onRemoteHomeButtonClicked();
    void onRemoteBackButtonClicked();
    void onRemoteForwardButtonClicked();
    void onRemoteRefreshButtonClicked();
    void onRemoteCopyButtonClicked();
    void onRemoteItemClicked(const QModelIndex& idx);
    void onRemoteItemDbClicked(const QModelIndex& idx);
    void onRemoteContextMenuRequested(const QPoint &pos);
    void onRemoteCreateDirectory();
    void onRemoteFileRefresh();
    void onRemoteFileDownload();
    void onRemoteDirectoryReload();
    void onRemoteEntryInfoListResult(qint8 err, const QByteArray& path, const QVariantList& files);

    void onListFilesResult(qint8 err, const QByteArray& path, const QByteArrayList& files);
    void onMkdirResult(qint8 err, const QByteArray& msg);

    // transfer
    void onTransferContextMenuRequested(const QPoint &pos);
    void onTransferStart();
    void onTransferRestart();
    void onTransferStop();
    void onTransferStartAll();
    void onTransferStopAll();
    void onTransferRemove();
    void onTransferRemoveAll();
    void onTaskUpdate(const QModelIndex& idx);

protected:
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
private:
    bool listFiles(const QString& path, QStringList& all, int maxCount = 50);
private:
    QString m_name;
    QString m_pathTask;

    QPointer<QSplitter> m_root;
    QPointer<QTreeView> m_transfer;
    QPointer<QTreeView> m_local;
    QPointer<QLineEdit> m_localEdit;
    QPointer<QTreeView> m_remote;
    QPointer<QLineEdit> m_remoteEdit;
    QPointer<QKxFtpLocalModel> m_modelLocal;
    QPointer<QKxFtpRemoteModel> m_modelRemote;
    QPointer<QKxFtpTransferModel> m_modelTransfer;

    QPointer<QPushButton> m_btnHome, m_btnReturn, m_btnEnter;
    QPointer<QPushButton> m_btnHome2, m_btnReturn2, m_btnEnter2;

    QPointer<QKxFtpRequest> m_ftpRequest;
    QPointer<QTimer> m_timer;
};

#endif // QKXFTPTRANSFERWINDOW_H
