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

#include "qkxscriptlocalcommand.h"

#include <QProcess>

QKxScriptLocalCommand::QKxScriptLocalCommand(QObject *parent)
    : QObject(parent)
{

}

QString QKxScriptLocalCommand::command() const
{
    return m_command;
}

void QKxScriptLocalCommand::setCommand(const QString &cmd)
{
    if(m_command != cmd) {
        m_command = cmd;
        emit commandChanged();
    }
}

int QKxScriptLocalCommand::lastExitCode() const
{
    return m_lastExitCode;
}

void QKxScriptLocalCommand::start()
{
    if(m_proc) {
        m_proc->kill();
        m_proc->deleteLater();
    }

    m_proc = new QProcess(this);
    QObject::connect(m_proc, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    QObject::connect(m_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError()));

    QObject::connect(m_proc, SIGNAL(readyReadStandardError()), this, SLOT(onErrorReadyRead()));
    QObject::connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyRead()));

    QObject::connect(m_proc, SIGNAL(error(QProcess::ProcessError)), m_proc, SLOT(deleteLater()));
    QObject::connect(m_proc, SIGNAL(finished(int)), m_proc, SLOT(deleteLater()));
    QObject::connect(m_proc, SIGNAL(destroyed(QObject*)), this, SLOT(onProcessDestroy()));
    m_proc->start(m_command);
    m_lastExitCode = -1;
    emit commandStart();
}

void QKxScriptLocalCommand::abort()
{
    if(m_proc) {
        m_proc->kill();
    }
    m_proc->deleteLater();
}

void QKxScriptLocalCommand::onFinished(int code)
{
    m_lastExitCode = -1;
    if(m_proc->exitStatus() == QProcess::NormalExit) {
        m_lastExitCode = m_proc->exitCode();
    }
}

void QKxScriptLocalCommand::onReadyRead()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    proc->setReadChannel(QProcess::StandardOutput);
    QByteArray buf = proc->readAll();
    emit dataArrived(buf);
}

void QKxScriptLocalCommand::onErrorReadyRead()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    proc->setReadChannel(QProcess::StandardError);
    QByteArray buf = proc->readAll();
    emit dataArrived(buf);
}

void QKxScriptLocalCommand::onError()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    QProcess::ProcessError err = proc->error();
    if(err == QProcess::FailedToStart) {
        emit dataArrived("Failed to start.");
    }else if(err == QProcess::Crashed) {
        emit dataArrived("process crash.");
    }else if(err == QProcess::ReadError) {
        emit dataArrived("Failed to read.");
    }else if(err == QProcess::WriteError) {
        emit dataArrived("Failed to write.");
    }else{
        emit dataArrived("Unknow error.");
    }
    proc->deleteLater();
}

void QKxScriptLocalCommand::onProcessDestroy()
{
    emit commandFinished();
}
