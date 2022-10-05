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

#ifndef QWOHOSTSIMPLELIST_H
#define QWOHOSTSIMPLELIST_H

#include "qwoglobal.h"

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoHostList;
}

class QWoHostListModel;
class QSortFilterProxyModel;
class QAbstractItemModel;

class QWoHostSimpleList : public QDialog
{
    Q_OBJECT

public:
    explicit QWoHostSimpleList(EHostType type, QWidget *parent = nullptr);
    ~QWoHostSimpleList();

    bool result(HostInfo *phi);
private slots:
    void onEditTextChanged(const QString& txt);
    void onListItemDoubleClicked(const QModelIndex& item);
    void onOpenSelectSessions();
private:
    void init();
private:
    Ui::QWoHostList *ui;
    QPointer<QAbstractItemModel> m_model;
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QString m_result;
};

#endif // QWOHOSTSIMPLELIST_H
