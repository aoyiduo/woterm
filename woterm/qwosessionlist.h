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

#pragma once

#include <QWidget>
#include <QPointer>
#include "qwoglobal.h"

class QLineEdit;
class QListWidget;
class QPushButton;
class QWoHostListModel;
class QSortFilterProxyModel;
class QWoListView;
class QMenu;
class QPlainTextEdit;

class QWoSessionList : public QWidget
{
    Q_OBJECT
public:
    explicit QWoSessionList(QWidget *parent=nullptr);
    virtual ~QWoSessionList();
signals:
    void aboutToClose(QCloseEvent* event);
    void readyToConnect(const QString& target,int type);
private:
    void init();
    void refreshList();
    void autoOpen();
private slots:
    void onReloadSessionList();
    void onEditTextChanged(const QString& txt);
    void onListItemDoubleClicked(const QModelIndex& item);
    void onListCurrentItemChanged(const QModelIndex& item);
    void onListReturnKeyPressed();
    void onTimeout();
    void onEditReturnPressed();
    void onListViewItemOpenSsh();
    void onListViewItemOpenSftp();
    void onListViewItemOpenTelnet();
    void onListViewItemOpenRLogin();
    void onListViewItemOpenMstsc();
    void onListViewItemOpenVnc();
    void onListViewItemOpenSerialPort();
    void onListViewItemReload();
    void onListViewItemModify();
    void onListViewItemAdd();
    void onListViewItemDelete();
private:
    bool handleListViewContextMenu(QContextMenuEvent *ev);
    bool handleListViewMouseButtonPress(QMouseEvent *ev);
private:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
private:
    QPointer<QWoHostListModel> m_model;
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QPointer<QLineEdit> m_input;
    QPointer<QWoListView> m_list;
    QPointer<QPlainTextEdit> m_info;
    int m_countLeft;
    QPointer<QMenu> m_menu;
    QPointer<QAction> m_itemOpen;
};
