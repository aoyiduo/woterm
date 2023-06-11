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

#include "qkxscriptremotecommand.h"

#include "qwossh.h"
#include "qwoutils.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>

QKxScriptRemoteCommand::QKxScriptRemoteCommand(QObject *parent)
    : QKxScriptCommand(parent)
    , m_enterSymbols("\n")
    , m_keyboardInteractive(false)
{

}

QKxScriptRemoteCommand::~QKxScriptRemoteCommand()
{

}

QString QKxScriptRemoteCommand::shellCommand() const
{
    return m_command;
}

void QKxScriptRemoteCommand::setShellCommand(const QString &cmd)
{
    //qDebug() << "setShellCommand" << cmd;
    if(m_command != cmd) {
        m_command = cmd;
        emit commandChanged();
    }
}

QString QKxScriptRemoteCommand::enterSymbols() const
{
    return m_enterSymbols;
}

void QKxScriptRemoteCommand::setEnterSymbols(const QString &key)
{
    if(m_enterSymbols != key) {
        m_enterSymbols = key;
        emit enterSymbolsChanged();
    }
}

void QKxScriptRemoteCommand::setKeyboardInteractiveWithTerminal(bool on)
{
    m_keyboardInteractive = on;
}

void QKxScriptRemoteCommand::onDataArrived(const QByteArray &buf)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    emit dataArrived(channel->targetHost(), buf);
}

void QKxScriptRemoteCommand::onSendData(const QByteArray &data)
{
    if(m_shell && m_keyboardInteractive) {
        m_shell->write(data);
    }
}

QWoSshChannel *QKxScriptRemoteCommand::channelGet()
{
    QWoSshShell *shell = QWoSshFactory::instance()->createShell(true);
    QObject::connect(shell, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));
    shell->updateSize(240, 160);

    QQmlEngine *engine = qmlEngine(this);
    if(engine) {
        QObject *terminal = engine->property("terminal").value<QObject*>();
        if(terminal) {
            QObject::connect(terminal, SIGNAL(sendData(QByteArray)), this, SLOT(onSendData(QByteArray)));
        }
    }

    m_shell = shell;
    return shell;
}

bool QKxScriptRemoteCommand::runTask(QWoSshChannel *channel)
{
    QWoSshShell *shell = qobject_cast<QWoSshShell*>(channel);
    QString cmd = m_command;
    if(!cmd.endsWith(m_enterSymbols)) {
        cmd.append(m_enterSymbols);
    }
    shell->write(cmd.toUtf8());
    return true;
}

int QKxScriptRemoteCommand::lastExitCode(QWoSshChannel *channel)
{
    QWoSshShell *shell = qobject_cast<QWoSshShell*>(channel);
    return shell->lastCommandExitCode();
}

void QKxScriptRemoteCommand::runAbort()
{
    if(m_shell) {
        m_shell->write("\u0003");
    }
    QKxScriptCommand::runAbort();
}
