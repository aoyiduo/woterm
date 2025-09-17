/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QKXFILTERLISTVIEW_H
#define QKXFILTERLISTVIEW_H

#include <QPointer>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QEvent>

namespace Ui {
class QKxFilterListView;
}

class QLineEdit;
class QListView;
class QSortFilterProxyModel;
class QPushButton;
class QToolButton;

class QSessionButtonActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit QSessionButtonActionDelegate(QListView *tblView, QWidget *parent = 0);
    bool editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const;

signals:
    void sftpArrived(const QModelIndex& idx) const;
private:
    bool _editorEvent(QEvent *ev, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &idx);

    QRect buttonRect(const QStyleOptionViewItem &option) const;
private:
    QPointer<QWidget> m_parent;
    QPointer<QListView> m_listView;
    QToolButton *m_btnSftp;
    QToolButton *m_btnEdit;
    QPoint m_ptMouse;
    QEvent::Type m_event;
};


class QKxFilterListView : public QWidget
{
    Q_OBJECT

public:
    explicit QKxFilterListView(QLineEdit *input, QWidget *parent = nullptr);
    ~QKxFilterListView();

    Q_INVOKABLE void enableFilter(bool on);
    QListView *listView();
signals:
    void itemClicked(const QString& name, int type);
    void addClicked(const QString& name);

private slots:
    void onEditTextChanged(const QString& txt);
    void onListItemClicked(const QModelIndex &index);
    void onDelegateSftpArrived(const QModelIndex& idx);
private:
    virtual bool eventFilter(QObject *obj, QEvent *ev);
    Q_INVOKABLE void resetPosition();
    int minimizeHeight(int mh);
private:
    void handleShowEvent(QShowEvent *ev);
    void handleHideEvent(QHideEvent *ev);
    void handleMouseMoveEvent(QMouseEvent *ev);
    void handleMousePressEvent(QMouseEvent *ev);
private:
    Ui::QKxFilterListView *ui;
    QPointer<QLineEdit> m_input;
    bool m_filterEnable;
    QPointer<QSortFilterProxyModel> m_filter;
    QPointer<QWidget> m_grab;
    QString m_sftpFirstResponse;
};

#endif // QKXFILTERLISTVIEW_H
