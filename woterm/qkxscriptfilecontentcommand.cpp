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

#include "qkxscriptfilecontentcommand.h"

#include "qwossh.h"
#include "qwoutils.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

QKxScriptFileContentCommand::QKxScriptFileContentCommand(QObject *parent)
    : QKxScriptCommand(parent)
    , m_fileMaxSize(1024 * 1024)
    , m_fileRead(true)
    , m_opcode(-1)
{

}

QKxScriptFileContentCommand::~QKxScriptFileContentCommand()
{

}

QString QKxScriptFileContentCommand::filePath() const
{
    return m_filePath;
}

void QKxScriptFileContentCommand::setFilePath(const QString &path)
{
    m_filePath = path;
}

bool QKxScriptFileContentCommand::fileRead()
{
    return m_fileRead;
}

void QKxScriptFileContentCommand::setFileRead(bool on)
{
    m_fileRead = on;
}

int QKxScriptFileContentCommand::maxFileSize() const
{
    return m_fileMaxSize;
}

void QKxScriptFileContentCommand::setMaxFileSize(int size)
{
    m_fileMaxSize = size;
}

QByteArray QKxScriptFileContentCommand::fileContent() const
{
    return m_content;
}

void QKxScriptFileContentCommand::setFileContent(const QByteArray &all)
{
    if(all.size() > m_fileMaxSize) {
        emit errorArrived("", tr("the file content exceeds max file size, maybe you should reset the max file size first."));
        return;
    }
    if(m_content != all) {
        m_content = all;
        emit contentChanged();
    }
}


void QKxScriptFileContentCommand::onSftpCommandStart(int type, const QVariantMap &userData)
{

}

void QKxScriptFileContentCommand::onSftpCommandFinish(int type, const QVariantMap &userData)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(sender());
    QString host = sftp->targetHost();
    // not implement.
    QString reason = userData.value("reason").toString();
    if(reason == "fatal") {
        emit errorArrived(host, tr("Failed to download the backup file."));
        return;
    }
    int code = userData.value("code").toInt();
    QString error = userData.value("errorString").toString();
    if(type == MT_FTP_READ_FILE_CONTENT) {
        QVariantMap file = userData.value("fileContent").toMap();
        if(file.isEmpty()) {
            if(code == 2) {
                emit errorArrived(host, tr("No relevant version information was found."));
            }else{
                emit errorArrived(host, tr("Unknow error was found."));
            }
        }else{
            m_content = file.value("content").toByteArray();
        }
    }
    if(!error.isEmpty()) {
        emit errorArrived(host, error);
    }
    m_opcode = code;
    sftp->stop();
}

QWoSshChannel *QKxScriptFileContentCommand::channelGet()
{
    QWoSshFtp *sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onSftpCommandStart(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onSftpCommandFinish(int,QVariantMap)));
    return sftp;
}

bool QKxScriptFileContentCommand::runTask(QWoSshChannel *channel)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(channel);
    QVariantMap dm;
    dm.insert("remote", m_filePath);
    dm.insert("command", "fileContent");
    if(m_fileRead) {
        m_content.clear();
        sftp->fileContent(m_filePath, 0, m_fileMaxSize, dm);
        return true;
    }
    sftp->writeFileContent(m_filePath, m_content, dm);
    m_opcode = -1;
    return true;
}

int QKxScriptFileContentCommand::lastExitCode(QWoSshChannel *channel)
{
    return m_opcode == 0 ? 0 : 1;
}
