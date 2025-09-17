/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWODBPOWERRESTOREDIALOG_H
#define QWODBPOWERRESTOREDIALOG_H

#include "qwoglobal.h"

#include <QDialog>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QStyledItemDelegate>
#include <QEvent>

namespace Ui {
class QWoDBPowerRestoreDialog;
}

class QTableView;

class QDBMergeActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit QDBMergeActionDelegate(QTableView *tblView, QWidget *parent = 0);
    bool editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const;

signals:
    void actionArrived(int action, const QModelIndex& idx) const;
private:
    bool _editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);

private:
    QPointer<QWidget> m_parent;
    QPointer<QTableView> m_tblView;
    QPushButton *m_btnReplace;
    QPushButton *m_btnAdd;
    QPushButton *m_btnRemove;
    QPoint m_ptMouse;
    QEvent::Type m_event;
};

class QWoDBSftpDownSync;
class QWoDBMergeModel;
class QWoDBPowerRestoreDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDBPowerRestoreDialog(QWidget *parent = nullptr);
    ~QWoDBPowerRestoreDialog();


private slots:
    void onCurrentBackupIndexChanged();
    void onCurrentProjectIndexChanged();
    void onAdjustLayout();
    void onSftpDetailButtonClicked();
    void onDecryptAgainClicked();
    void onButtonApplyClicked();

    void onMergeWidgetShow();
    void onTypeWidgetShow();
    void onBackupListClicked();
    void onGroupModeClicked();
    void onLocalFileBrowserClicked();


    void onActionArrived(int action, const QModelIndex& idx);

    void onSyncInfoArrived(int action, int err, const QString& errDesc);
    void onSyncFetchFinished(int err, const QString& fileName);
private:    
    Q_INVOKABLE void fetchLocal();
    void resetSftpUrl();
    bool decryptFile(const QString &fileNameSrc, const QString& fileNameDst);
    bool decrypt(const QByteArray& in, const QByteArray& type, const QByteArray& key, QByteArray& out);
    bool runMerge(const QString& fileName);
    MergeInfo runMergeList(const QList<QVariantMap>& local, const QList<QVariantMap>& remote) const;
    void showMergeResult();
    bool dbToMap(const QString& filePath, QMap<QString, QList<QVariantMap>>& all);
    Q_INVOKABLE void init();
private:
    Ui::QWoDBPowerRestoreDialog *ui;
    QPointer<QWoDBSftpDownSync> m_sync;
    QPointer<QWoDBMergeModel> m_modelServer, m_modelGroup, m_modelIdentity, m_modelCurrent;
    bool m_sftpMode;
    QString m_pathTemp;
    QString m_cryptType, m_cryptKey;

    QMap<QString, QList<QVariantMap>> m_local, m_remote;
};

#endif // QWODBPOWERRESTOREDIALOG_H
