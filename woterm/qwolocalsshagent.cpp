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

#include "qwolocalsshagent.h"

#include "qwosetting.h"

#include <QTcpSocket>
#include <QLocalSocket>
#include <QTcpServer>
#include <QDebug>
#include <QProcessEnvironment>
#include <QCoreApplication>

QWoLocalSshAgentClient::QWoLocalSshAgentClient(const QString& sshAuthSock, QTcpSocket *tcp, QObject *parent)
    : QObject(parent)
    , m_tcp(tcp)
{
    QObject::connect(tcp, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    QObject::connect(tcp, SIGNAL(readyRead()), this, SLOT(onTcpReadyRead()));
    m_local = new QLocalSocket(this);
    QObject::connect(m_local, SIGNAL(readyRead()), this, SLOT(onLocalReadyRead()));
    QObject::connect(m_local, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    QObject::connect(m_local, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(deleteLater()));
    m_local->connectToServer(sshAuthSock);
}

QWoLocalSshAgentClient::~QWoLocalSshAgentClient()
{
    if(m_tcp) {
        m_tcp->deleteLater();
    }
    if(m_local) {
        m_local->deleteLater();
    }
}

void QWoLocalSshAgentClient::onTcpReadyRead()
{
    QByteArray all = m_tcp->readAll();
    m_local->write(all);
}

void QWoLocalSshAgentClient::onLocalReadyRead()
{
    QByteArray all = m_local->readAll();
    m_tcp->write(all);
}

QWoLocalSshAgentServer::QWoLocalSshAgentServer(QObject *parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    QObject::connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    m_server->listen(QHostAddress("127.0.0.1"), 0);
}

QWoLocalSshAgentServer *QWoLocalSshAgentServer::instance()
{
    static QPointer<QWoLocalSshAgentServer> gLocalSshAgent;
    if(gLocalSshAgent) {
        return gLocalSshAgent;
    }
    gLocalSshAgent = new QWoLocalSshAgentServer(QCoreApplication::instance());
    QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), gLocalSshAgent, SLOT(deleteLater()));
    return gLocalSshAgent;
}

int QWoLocalSshAgentServer::port() const
{
    return m_server->serverPort();
}

QString QWoLocalSshAgentServer::host() const
{
    return m_server->serverAddress().toString();
}

void QWoLocalSshAgentServer::onNewConnection()
{
    QString addr = QWoSetting::lastLocalSshAgentAddress();
    if(addr.isEmpty()) {
        addr = QWoSetting::sshAuthSockDefault();
    }
    while(m_server->hasPendingConnections()) {
        QTcpSocket *tcp = m_server->nextPendingConnection();
        if(addr.isEmpty()) {
            tcp->close();
            tcp->deleteLater();
            continue;
        }
        QWoLocalSshAgentClient *client = new QWoLocalSshAgentClient(addr, tcp, this);
        QObject::connect(client, SIGNAL(destroyed()), this, SLOT(onClientDestroyed()));
        m_clients.append(client);
    }
}

void QWoLocalSshAgentServer::onClientDestroyed()
{
    QWoLocalSshAgentClient *client = qobject_cast<QWoLocalSshAgentClient*>(sender());
    m_clients.removeAll(client);
}

