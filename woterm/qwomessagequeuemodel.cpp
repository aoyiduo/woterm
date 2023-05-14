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
