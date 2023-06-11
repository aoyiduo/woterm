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

#include "qkxscriptbigscriptcommand.h"


#include "qwossh.h"
#include "qwoutils.h"

#include <QFile>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>
#include <QCryptographicHash>

QKxScriptBigScriptCommand::QKxScriptBigScriptCommand(QObject *parent)
    : QKxScriptCommand(parent)
    , m_enterSymbols("\n")
    , m_keyboardInteractive(false)
{

}

QKxScriptBigScriptCommand::~QKxScriptBigScriptCommand()
{
    if(m_shell) {
        QWoSshFactory::instance()->release(m_shell);
    }
}

QString QKxScriptBigScriptCommand::scriptContent() const
{
    return m_scriptContent;
}

void QKxScriptBigScriptCommand::setScriptContent(const QString &content)
{
    if(m_scriptContent != content) {
        m_scriptContent = content;
        emit scriptChanged();
    }
}

QString QKxScriptBigScriptCommand::enterSymbols() const
{
    return m_enterSymbols;
}

void QKxScriptBigScriptCommand::setEnterSymbols(const QString &key)
{
    if(m_enterSymbols != key) {
        m_enterSymbols = key;
        emit enterSymbolsChanged();
    }
}

bool QKxScriptBigScriptCommand::loadFromFile(const QString &fileName)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly)) {
        return false;
    }
    QByteArray all = f.readAll();
    setScriptContent(all);
    return true;
}

bool QKxScriptBigScriptCommand::saveToFile(const QString &fileName)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadWrite)) {
        return false;
    }
    f.write(m_scriptContent.toUtf8());
    return true;
}

void QKxScriptBigScriptCommand::setKeyboardInteractiveWithTerminal(bool on)
{
    m_keyboardInteractive = on;
}

void QKxScriptBigScriptCommand::onSftpCommandStart(int type, const QVariantMap &userData)
{
    // not implement.
}

void QKxScriptBigScriptCommand::onSftpCommandFinish(int type, const QVariantMap &userData)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(sender());
    QString host = sftp->targetHost();
    // not implement.
    QString reason = userData.value("reason").toString();
    if(reason == "fatal") {
        emit errorArrived(host, tr("Failed to Run the script."));
        return;
    }
    int code = userData.value("code").toInt();
    QString error = userData.value("errorString").toString();
    if(!error.isEmpty()) {
        emit errorArrived(host, error);
    }
    if(code == 0) {
        // disconnect sftp, then shell reconnect it.
        QObject::disconnect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));

        QString fileRemote = userData.value("fileRemote").toString();
        QWoSshShell *shell = QWoSshFactory::instance()->createShell(true);
        if(m_shell != shell) {
            QWoSshFactory::instance()->release(m_shell);
        }
        m_shell = shell;

        QQmlEngine *engine = qmlEngine(this);
        if(engine) {
            QObject *terminal = engine->property("terminal").value<QObject*>();
            if(terminal) {
                QObject::connect(terminal, SIGNAL(sendData(QByteArray)), this, SLOT(onSendData(QByteArray)));
            }
        }

        QObject::connect(shell, SIGNAL(dataArrived(QByteArray)), this, SLOT(onShellDataArrived(QByteArray)));
        QObject::connect(shell, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
        QObject::connect(shell, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
        QObject::connect(shell, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
        shell->updateSize(240, 160);
        shell->start(host, groupId(host));
        QString cmd = QString("chmod a+x %1 && %1 && rm -f %1 || rm -f %1%2").arg(fileRemote, m_enterSymbols);
        shell->write(cmd.toUtf8());
    }else{
        m_opcode = code;
        if(m_shell) {
            QWoSshFactory::instance()->release(m_shell);
            m_shell = nullptr;
        }
        m_sftp->stop();
    }
}

void QKxScriptBigScriptCommand::onShellDataArrived(const QByteArray &buf)
{
    QWoSshChannel *channel = qobject_cast<QWoSshChannel*>(sender());
    emit dataArrived(channel->targetHost(), buf);
}

void QKxScriptBigScriptCommand::onSendData(const QByteArray &data)
{
    if(m_shell && m_keyboardInteractive) {
        m_shell->write(data);
    }
}

QWoSshChannel *QKxScriptBigScriptCommand::channelGet()
{
    QWoSshFtp *sftp = QWoSshFactory::instance()->createSftp();
    m_sftp = sftp;
    QObject::connect(sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onSftpCommandStart(int,QVariantMap)));
    QObject::connect(sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onSftpCommandFinish(int,QVariantMap)));
    return sftp;
}

bool QKxScriptBigScriptCommand::runTask(QWoSshChannel *channel)
{
    QWoSshFtp *sftp = qobject_cast<QWoSshFtp*>(channel);
    QVariantMap dm;
    QByteArray hash = QCryptographicHash::hash(m_scriptContent.toUtf8(), QCryptographicHash::Md5);
    QString hex = hash.toHex();
    QString fileRemote = QString("~/.woterm_playbook_script_%1").arg(hex.mid(16));
    dm.insert("fileRemote", fileRemote);
    dm.insert("command", "fileContent");
    sftp->writeFileContent(fileRemote, m_scriptContent.toUtf8(), dm);
    m_opcode = -1;
    return true;
}

int QKxScriptBigScriptCommand::lastExitCode(QWoSshChannel *channel)
{
    if(m_shell) {
        return m_shell->lastCommandExitCode();
    }
    return m_opcode == 0 ? 0 : 1;
}

void QKxScriptBigScriptCommand::runAbort()
{
    if(m_shell) {
        m_shell->stop();
    }
    QKxScriptCommand::runAbort();
}
