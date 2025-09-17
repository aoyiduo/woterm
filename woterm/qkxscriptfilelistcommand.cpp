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

#include "qkxscriptfilelistcommand.h"

#include "qwossh.h"
#include "qwoutils.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

QKxScriptFileListCommand::QKxScriptFileListCommand(QObject *parent)
    : QKxScriptCommand(parent)
{

}

QKxScriptFileListCommand::~QKxScriptFileListCommand()
{

}

QString QKxScriptFileListCommand::path() const
{
    return m_path;
}

void QKxScriptFileListCommand::setPath(const QString &path)
{
    if(m_path != path) {
        m_path = path;
        emit pathChanged();
    }
}

QVariantList QKxScriptFileListCommand::fileInfos() const
{
    return m_fileLists;
}

QStringList QKxScriptFileListCommand::fileNames() const
{
    QStringList names;
    for(int i = 0; i < m_fileLists.length(); i++) {
        QMap<QString, QVariant> mdata = m_fileLists.at(i).toMap();
        QString name = mdata.value("name").toString();
        if(name == "." || name == "..") {
            continue;
        }
        names.append(name);
    }
    return names;
}

void QKxScriptFileListCommand::onSftpCommandStart(int type, const QVariantMap &userData)
{

}

void QKxScriptFileListCommand::onSftpCommandFinish(int type, const QVariantMap &userData)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(sender());
    QString host = sftp->targetHost();
    // not implement.
    QString reason = userData.value("reason").toString();
    if(reason == "abort") {
        emit errorArrived(host, "list file abort.");
    }else if(reason == "error") {
        QString err = userData.value("errorString").toString();
        if(!err.isEmpty()) {
            emit errorArrived(host, err);
        }
    }
    sftp->stop();
}

void QKxScriptFileListCommand::onDirOpen(const QString &path, const QList<QVariant> &v, const QVariantMap &userData)
{
    m_fileLists = v;
    m_dirOpenOk = true;
}

QWoSshChannel *QKxScriptFileListCommand::channelGet()
{
    QWoSshFtp *sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onSftpCommandStart(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onSftpCommandFinish(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(dirOpen(QString,QVariantList,QVariantMap)), this, SLOT(onDirOpen(QString,QVariantList,QVariantMap)));
    return sftp;
}

bool QKxScriptFileListCommand::runTask(QWoSshChannel *channel)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(channel);
    sftp->openDir(m_path);
    m_fileLists.clear();
    m_dirOpenOk = false;
    return true;
}

int QKxScriptFileListCommand::lastExitCode(QWoSshChannel *channel)
{
    return m_dirOpenOk ? 0 : 1;
}
