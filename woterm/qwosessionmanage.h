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

#ifndef QWOSESSIONMANAGE_H
#define QWOSESSIONMANAGE_H

#include "qwoglobal.h"

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>

namespace Ui {
class QWoSessionManage;
}

class QKxTermWidget;
class QStringListModel;
class QWoHostTreeModel;
class QWoHostListModel;
class QWoSortFilterProxyModel;
class QWoTreeView;


class QWoSessionManage : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionManage(QWidget *parent = nullptr);
    ~QWoSessionManage();

signals:
    void readyToConnect(const QString& host, int type);
    void readyToConnect(const QStringList& host, bool diff);

private slots:
    void onEditTextChanged(const QString &txt);
    void onSshConnectReady();
    void onSftpConnectReady();
    void onTelnetConnectReady();
    void onRLoginConnectReady();
    void onMstscConnectReady();
    void onVncConnectReady();
    void onSerialPortConnectReady();
    void onOpenReady();
    void onDeleteReady();
    void onModifyReady();
    void onNewReady();
    void onCopyReady();
    void onImportReady();
    void onTreeViewOpenInSamePage();
    void onTreeViewOpenInDifferentPage();
    void onTreeItemSelected(const QModelIndex& idx);
    void onTreeItemDoubleClicked(const QModelIndex& idx);
    void onTreeModelSwitch();
private:
    Q_INVOKABLE void refreshList();
    Q_INVOKABLE void resizeHeader();
    Q_INVOKABLE void restoreSessionsExpandState();
    Q_INVOKABLE void saveSessionsExpandState();
    bool handleTreeViewContextMenu(QContextMenuEvent *ev);

    Q_INVOKABLE void sessionEditLater(const QString& sessionName);
    Q_INVOKABLE void scrollToSession(const QString& sessionName);
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::QWoSessionManage *ui;
    QPointer<QWoHostListModel> m_listModel;
    QPointer<QWoHostTreeModel> m_treeModel;
    QPointer<QAbstractItemModel> m_model;
    QPointer<QWoSortFilterProxyModel> m_proxyModel;
    QPointer<QWoTreeView> m_tree;
};

#endif // QWOSESSIONPROPERTY_H
