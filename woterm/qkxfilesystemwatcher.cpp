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

#include "qkxfilesystemwatcher.h"

#include <QFileInfo>
#include <QTimer>

QKxFileSystemWatcher::QKxFileSystemWatcher(QObject *parent)
    : QObject(parent)
{

}

QKxFileSystemWatcher::~QKxFileSystemWatcher()
{

}

bool QKxFileSystemWatcher::addPath(const QString &file)
{
    QFileInfo fi(file);
    if(!fi.isFile()) {
        return false;
    }
    m_files.insert(file, fi.lastModified());

    if(m_timer == nullptr) {
        m_timer = new QTimer(this);
        QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onFileCheckTimeout()));
    }
    if(!m_timer->isActive()) {
        m_timer->start(1000);
    }
    return true;
}

void QKxFileSystemWatcher::removePath(const QString &file)
{
    m_files.take(file);
}

QStringList QKxFileSystemWatcher::files() const
{
    return m_files.keys();
}

void QKxFileSystemWatcher::onFileCheckTimeout()
{
    for(auto it = m_files.begin(); it != m_files.end(); ) {
        QString file = it.key();
        QDateTime dt = it.value();
        QFileInfo fi(file);
        if(fi.lastModified() != dt) {
            it = m_files.erase(it);
            emit fileChanged(file);
        }else{
            it++;
        }
    }
    if(m_files.isEmpty()) {
        m_timer->stop();
    }
}
