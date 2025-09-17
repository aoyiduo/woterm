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

#include "qwomessagequeuemodel.h"

#include <QSize>
QWoMessageQueueModel::QWoMessageQueueModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

void QWoMessageQueueModel::warning(const QString &title, const QString &content)
{

}


int QWoMessageQueueModel::rowCount(const QModelIndex &parent) const
{
    return m_queue.length();
}

int QWoMessageQueueModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QWoMessageQueueModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0 || row >= m_queue.length()) {
        return QVariant();
    }
    const MessageData& md = m_queue.at(row);
    if(role == Qt::DecorationRole) {
        if(md.type == EMTInformation) {
            return QVariant();
        }else if(md.type == EMTWarning) {
            return QVariant();
        }
        return QVariant();
    }
    if(role == Qt::DisplayRole) {
        return md.content;
    }
    if(role == Qt::SizeHintRole) {
        return QSize(-1, 30);
    }
    if(role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant QWoMessageQueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}
