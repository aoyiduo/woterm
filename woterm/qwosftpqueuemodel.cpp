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

#include "qwosftpqueuemodel.h"

#include <QSize>
#include <QPixmap>
#include <QGuiApplication>
#include <QFontMetrics>

#define BUTTON_SPACE    (40)

QWoSftpQueueModel::QWoSftpQueueModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_maxTextWidth(0)
    , m_maxTextHeight(0)
{
    m_font = QGuiApplication::font();
    m_uploadIcon = QIcon(QPixmap(":/woterm/resource/skin/upload.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_downIcon = QIcon(QPixmap(":/woterm/resource/skin/download.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_dirIcon = QIcon(QPixmap(":/woterm/resource/skin/dirs.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
    m_fileIcon = QIcon(QPixmap(":/woterm/resource/skin/file.png").scaled(18, 24, Qt::KeepAspectRatio ,Qt::SmoothTransformation));
}

bool QWoSftpQueueModel::append(const TaskInfo &ti)
{
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TaskInfo& t){
        return ti.local == t.local || ti.remote == t.remote;
    });
    if(it != m_tasks.end()) {
        return false;
    }
    beginResetModel();
    m_tasks.append(ti);
    QFontMetrics fm(m_font);
    int w = fm.width(tr("Local:") + ti.local);
    int w2 = fm.width(tr("Remote:")+ti.remote);
    if(w < w2) {
        w = w2;
    }
    if(w > m_maxTextWidth) {
        m_maxTextWidth = w;
    }
    TaskInfo& tmp = m_tasks.last();
    tmp.textWidth = w;
    if(m_maxTextHeight < fm.height()){
        m_maxTextHeight = fm.height();
    }
    endResetModel();
    return true;
}

int QWoSftpQueueModel::maxColumnWidth()
{
    return m_maxTextWidth + BUTTON_SPACE;
}

bool QWoSftpQueueModel::isEmpty() const
{
    return m_tasks.isEmpty();
}

TaskInfo QWoSftpQueueModel::takeFirst()
{
    beginResetModel();
    const TaskInfo& ti = m_tasks.takeFirst();
    QFontMetrics fm(m_font);

    int w = fm.width(tr("Local:") + ti.local);
    int w2 = fm.width(tr("Remote:")+ti.remote);
    if(w < w2) {
        w = w2;
    }
    if(w == m_maxTextWidth) {
        m_maxTextWidth  = 0;
        for(int i = 0; i < m_tasks.length(); i++) {
            const TaskInfo& tmp = m_tasks.at(i);
            if(tmp.textWidth > m_maxTextWidth) {
                m_maxTextWidth = tmp.textWidth;
            }
        }
        if(m_maxTextWidth == 0) {
            m_maxTextWidth = 100;
        }
    }
    endResetModel();
    return ti;
}

void QWoSftpQueueModel::removeTask(int tid)
{
    beginResetModel();
    QFontMetrics fm(m_font);
    for(auto it = m_tasks.begin(); it != m_tasks.end(); it++) {
        const TaskInfo& tmp = *it;
        if(tmp.taskId == tid) {
            m_tasks.erase(it);
            break;
        }
    }
    m_maxTextWidth  = 0;
    for(int i = 0; i < m_tasks.length(); i++) {
        const TaskInfo& tmp = m_tasks.at(i);
        if(tmp.textWidth > m_maxTextWidth) {
            m_maxTextWidth = tmp.textWidth;
        }
    }
    if(m_maxTextWidth == 0) {
        m_maxTextWidth = 100;
    }
    endResetModel();
}

void QWoSftpQueueModel::removeAll()
{
    beginResetModel();
    m_tasks.clear();
    endResetModel();
}

int QWoSftpQueueModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return m_tasks.count();
}

QVariant QWoSftpQueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        if(section == 0) {
            return tr("Type");
        }else if(section == 1) {
            return tr("FileName");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant QWoSftpQueueModel::data(const QModelIndex &idx, int role) const
{
    //qDebug() << "HostListModel" << role;
    if (idx.row() < 0 || idx.row() >= m_tasks.size()) {
        return QVariant();
    }
    if (!idx.isValid()){
        return QVariant();
    }
    const TaskInfo& ti = m_tasks.at(idx.row());

    if(role == Qt::FontRole) {
        return m_font;
    }

    if(role == Qt::ToolTipRole) {
        QString label = tr("Local:") + ti.local+"\n"+tr("Remote:")+ti.remote;
        return label;
    }

    if(role == Qt::SizeHintRole) {
        QFontMetrics fm(m_font);
        int column = idx.column();
        if(column == 0) {
            return QSize(40, 24);
        }
        if(column == 1) {            
            return QSize(m_maxTextWidth + BUTTON_SPACE, m_maxTextHeight * 3);
        }
        return QSize(-1, 24);
    }

    if(role == ROLE_INDEX) {
        return idx.row();
    }

    if(role == ROLE_TASKINFO) {
        QVariant v;
        v.setValue(ti);
        return v;
    }

    if (role == Qt::DisplayRole) {
        if(idx.column() == 0) {
            return QVariant();
        }
        if(idx.column() == 1) {
            QString label = tr("Local:") + ti.local+"\n"+tr("Remote:")+ti.remote;
            return label;
        }
        return QVariant();
    }
    return QVariant();
}

bool QWoSftpQueueModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

Qt::ItemFlags QWoSftpQueueModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid()){
        return QAbstractListModel::flags(idx) | Qt::ItemIsDropEnabled;
    }
    return QAbstractListModel::flags(idx)| Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool QWoSftpQueueModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)){
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r){
        m_tasks.insert(row, TaskInfo());
    }

    endInsertRows();

    return QAbstractListModel::insertRows(row, count, parent);
}

bool QWoSftpQueueModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent)){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = m_tasks.begin() + row;
    m_tasks.erase(it, it + count);

    endRemoveRows();

    return true;
}

void QWoSftpQueueModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractListModel::sort(column, order);
}

int QWoSftpQueueModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

Qt::DropActions QWoSftpQueueModel::supportedDropActions() const
{
    return QAbstractListModel::supportedDropActions();
}

void QWoSftpQueueModel::test()
{
    QString path = QCoreApplication::applicationDirPath();
    for(int i = 0; i < 10; i++) {
        TaskInfo ti;
        ti.isDir = i % 2;
        ti.isDown = i % 2;
        ti.local = path + QString("/local-%1").arg(i);
        ti.remote = path + QString("/remote-%1").arg(i);
        ti.taskId = 100 + i;
        append(ti);
    }
}
