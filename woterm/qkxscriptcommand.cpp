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

#include "qkxscriptcommand.h"

#include "qwossh.h"
#include "qwoutils.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

QKxScriptCommand::QKxScriptCommand(QObject *parent)
    : QObject(parent)
    , m_abort(false)
    , m_index(0)
{

}

QKxScriptCommand::~QKxScriptCommand()
{
    QWoSshFactory::instance()->release(m_channel);
}

QStringList QKxScriptCommand::targetHosts() const
{
    return m_hosts;
}

void QKxScriptCommand::setTargetHosts(const QStringList &hosts)
{
    if(m_hosts != hosts) {
        m_hosts = hosts;
        emit hostsChanged();
    }
}

void QKxScriptCommand::start()
{
    m_index = 0;
    m_abort = false;
    if(m_hosts.isEmpty()) {
        emit errorArrived("", tr("The hosts should not be empty."));
    }
    emit commandStart();
    runTask(m_index);
}

void QKxScriptCommand::abort()
{
    runAbort();
}

bool QKxScriptCommand::runTask(int idx)
{
    if(m_abort || idx < 0 || idx >= m_hosts.length()){
        emit commandFinished();
        return false;
    }
    QString host = m_hosts.at(idx);
    emit subcommandStart(host);
    QWoSshChannel *channel = channelGet();
    if(m_channel != channel) {
        QWoSshFactory::instance()->release(m_channel);
    }
    m_channel = channel;
    QObject::connect(channel, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(channel, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(channel, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(channel, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));

    channel->start(host, groupId(host));
    if(!runTask(channel)) {
        emit commandFinished();
        return false;
    }
    return true;
}

void QKxScriptCommand::runAbort()
{
    m_abort = true;
    if(m_channel) {
        m_channel->stop();
    }
}

void QKxScriptCommand::onConnectionFinished(bool ok)
{
}

/*
 * it's danger to modify the follow code. because the bigscriptcommand.
 * in bigscriptcommand, the chain command may disconnect and reconnect.
 *
*/
void QKxScriptCommand::onFinishArrived(int code)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());    
    if(channel == nullptr) {
        return;
    }
    int exitCode = lastExitCode(channel);
    emit subcommandFinished(channel->targetHost(), exitCode);
    m_index++;
    runTask(m_index);
}

void QKxScriptCommand::onErrorArrived(const QString &buf, const QVariantMap &userData)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    if(channel == nullptr) {
        return;
    }
    emit errorArrived(channel->targetHost(), buf);
}

void QKxScriptCommand::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    if(channel == nullptr) {
        return;
    }
    channel->stop();
}

int QKxScriptCommand::groupId(const QString &key)
{
    // try to reuse the factory.
    int gid = QWoSshFactory::instance()->groudId(key);
    if(gid > 0) {
        return gid;
    }
    return QWoUtils::gid();
}
