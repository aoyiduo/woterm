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

#ifndef QWOSFTPQUEUEMODEL_H
#define QWOSFTPQUEUEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QIcon>
#include <QFont>

#include "qwoglobal.h"

class QWoSftpQueueModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QWoSftpQueueModel(QObject *parent = nullptr);
    bool append(const TaskInfo& ti);
    int maxColumnWidth();
    bool isEmpty() const;
    TaskInfo takeFirst();
    void removeTask(int tid);
    void removeAll();
private:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::DropActions supportedDropActions() const override;
private:
    Q_INVOKABLE void test();
private:
    Q_DISABLE_COPY(QWoSftpQueueModel)
    QList<TaskInfo> m_tasks;
    QIcon m_dirIcon;
    QIcon m_fileIcon;
    QIcon m_uploadIcon;
    QIcon m_downIcon;
    QFont m_font;
    int m_maxTextWidth;
    int m_maxTextHeight;
};

#endif // QWOSFTPQUEUEMODEL_H
