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

#ifndef QWOSFTPTRANSFERWIDGET_H
#define QWOSFTPTRANSFERWIDGET_H

#include "qwoglobal.h"
#include "qwosftptask.h"

#include <QWidget>
#include <QPointer>
#include <QStyledItemDelegate>
#include <QPixmap>

namespace Ui {
class QWoSftpTransferWidget;
}

class QWoSshFtp;
class QWoSftpQueueModel;
class QWoTreeView;
class QPushButton;
class QWoSftpItemDelegate;
class QKxMessageBox;

class QWoSftpItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QWoSftpItemDelegate(QWidget *parent = 0);
signals:
    void removeArrived(int tid);
private slots:
    void onRemoveArrived();
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QPixmap m_dirIcon;
    QPixmap m_fileIcon;
    QPixmap m_uploadIcon;
    QPixmap m_downIcon;
    QPointer<QWidget> m_parent;
};

class QWoSftpTransferWidget : public QWidget
{
    Q_OBJECT
    enum EAbortExpected {
        EAbortNone = 0,
        EAbortIt = 1,
        EAbortAll = 2,
        EAbortAsStop = 3
    };

public:
    explicit QWoSftpTransferWidget(const QString &target, int gid, bool queueMode, QWidget *parent = 0);
    ~QWoSftpTransferWidget();
    bool isQueueMode();
    bool canAddTask();
    bool addTask(const QString& pathLocal, const QString& pathRemote, bool isDir, bool isDown, bool isAppend);
    void release();
signals:
    void commandStart(int type, const QVariantMap& userData);
    void commandFinish(int type, const QVariantMap& userData);
private slots:
    void onAdjustLayout();
    void onLocalFilePathBrowser();
    void onRemoteFilePathCopy();
    void onModelReset();


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

    void onInputArrived(const QString& host, const QString& prompt, bool visble);
    void onFileListArrived(const QVariantList& files, const QVariantMap& userData);

    void onTreeItemPressed(const QModelIndex &index);

private:
    QWoSftpTask *get();
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
    void resetAll();
    void outputFileList();
    Q_INVOKABLE void listFile(const QString& path);
    void setTaskLabel(const QString& local, const QString& remote);

    bool isPrivateSkinPath(const QString& path);
protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *e);
private:
    Ui::QWoSftpTransferWidget *ui;
    QPoint m_dragPosition;
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
    QPointer<QWoTreeView> m_tree;
    QPointer<QWoSftpTask> m_task;
    QPointer<QWoSftpQueueModel> m_taskModel;
    QPointer<QWoSftpItemDelegate> m_delegate;
};

#endif // QWOSFTPTRANSFERWIDGET_H
