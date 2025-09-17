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

#include "qkxscriptfiletransfercommand.h"

#include "qwossh.h"
#include "qwoutils.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

QKxScriptFileTransferCommand::QKxScriptFileTransferCommand(QObject *parent)
    : QKxScriptCommand(parent)
    , m_fileAppend(false)
    , m_fileDownload(false)
{

}

QKxScriptFileTransferCommand::~QKxScriptFileTransferCommand()
{

}

QString QKxScriptFileTransferCommand::fileLocal() const
{
    return m_fileLocal;
}

void QKxScriptFileTransferCommand::setFileLocal(const QString &file)
{
    if(m_fileLocal != file) {
        m_fileLocal = file;
        emit fileLocalChanged();
    }
}

QString QKxScriptFileTransferCommand::fileRemote() const
{
    return m_fileRemote;
}

void QKxScriptFileTransferCommand::setFileRemote(const QString &file)
{
    if(m_fileRemote != file) {
        m_fileRemote = file;
        emit fileRemoteChanged();
    }
}

bool QKxScriptFileTransferCommand::fileAppend() const
{
    return m_fileAppend;
}

void QKxScriptFileTransferCommand::setFileAppend(bool on)
{
    if(m_fileAppend != on) {
        m_fileAppend = on;
        emit fileAppendChanged();
    }
}

bool QKxScriptFileTransferCommand::fileDownload() const
{
    return m_fileDownload;
}

void QKxScriptFileTransferCommand::setFileDownload(bool on)
{
    if(m_fileDownload != on) {
        m_fileDownload = on;
        emit fileDownloadChanged();
    }
}

void QKxScriptFileTransferCommand::onTransferCommandStart(int type, const QVariantMap &userData)
{
    // not implement.
    m_transferProgress = 0;
}

void QKxScriptFileTransferCommand::onTransferCommandFinish(int type, const QVariantMap &userData)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(sender());
    QString host = sftp->targetHost();
    // not implement.
    QString reason = userData.value("reason").toString();
    if(reason == "abort") {
        emit errorArrived(host, "transfer abort.");
    }else if(reason == "error") {
        QString err = userData.value("errorString").toString();
        QString local = userData.value("local").toString();
        QString remote = userData.value("remote").toString();
        if(!err.isEmpty()) {
            emit errorArrived(host, err);
            emit errorArrived(host, QString("transfer parameter: local:%1-remote:%2").arg(local, remote));
        }
    }
    sftp->stop();
}

void QKxScriptFileTransferCommand::onTransferProgress(int type, int v, const QVariantMap &userData)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(sender());
    QString host = sftp->targetHost();
    m_transferProgress = v;
    emit progressArrived(host, v);
}

QWoSshChannel *QKxScriptFileTransferCommand::channelGet()
{
    QWoSshFtp *sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onTransferCommandStart(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onTransferCommandFinish(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(progress(int,int,QVariantMap)), this, SLOT(onTransferProgress(int,int,QVariantMap)));
    return sftp;
}

bool QKxScriptFileTransferCommand::runTask(QWoSshChannel *channel)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(channel);
    QVariantMap dm;
    dm.insert("remote", m_fileRemote);
    dm.insert("local", m_fileLocal);
    dm.insert("append", m_fileAppend);    
    dm.insert("command", m_fileDownload ? "download":"upload");
    m_transferProgress = 0;
    if(m_fileDownload) {
        sftp->download(m_fileRemote, m_fileLocal, m_fileAppend ? QWoSshFtp::TP_Append : QWoSshFtp::TP_Override, dm);
    }else{
        sftp->upload(m_fileLocal, m_fileRemote, m_fileAppend ? QWoSshFtp::TP_Append : QWoSshFtp::TP_Override, dm);
    }
    return true;
}

int QKxScriptFileTransferCommand::lastExitCode(QWoSshChannel *channel)
{
    return m_transferProgress == 100 ? 0 : m_transferProgress;
}
