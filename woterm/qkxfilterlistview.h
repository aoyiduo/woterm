/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QKXFILTERLISTVIEW_H
#define QKXFILTERLISTVIEW_H

#include <QPointer>
#include <QWidget>

namespace Ui {
class QKxFilterListView;
}

class QLineEdit;
class QListView;
class QSortFilterProxyModel;
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
};

#endif // QKXFILTERLISTVIEW_H
