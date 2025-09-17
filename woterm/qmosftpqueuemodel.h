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

#ifndef QMOSFTPQUEUEMODEL_H
#define QMOSFTPQUEUEMODEL_H

#include "qwoglobal.h"

#include <QAbstractListModel>
#include <QList>
#include <QFont>

class QMoSftpQueueModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ length NOTIFY lengthChanged)
public:
    explicit QMoSftpQueueModel(QObject *parent = nullptr);
    bool append(const TaskInfo& ti);
    bool isEmpty() const;
    TaskInfo takeFirst();
    Q_INVOKABLE void removeTask(int row);
    void removeAll();
    int length() const;

signals:
    void lengthChanged();

protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
private:
    QVariantMap convertToVariantMap(const TaskInfo& ti) const;
private:
    Q_DISABLE_COPY(QMoSftpQueueModel)
    QList<TaskInfo> m_tasks;
};

#endif // QMOSFTPQUEUEMODEL_H
