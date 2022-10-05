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

#include "qkxftpserverprivate.h"
#include "qkxftpclient.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QThread>

QKxFtpServerPrivate::QKxFtpServerPrivate(const QString &envName)
    : QObject(nullptr)
    , m_bClose(false)
    , m_envName(envName)
{
    m_worker = new QThread(nullptr);
    moveToThread(m_worker);
    m_worker->start();
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

QKxFtpServerPrivate::~QKxFtpServerPrivate()
{
    QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
    m_worker->wait();
    delete m_worker;
}

void QKxFtpServerPrivate::onNewConnection()
{
    QTcpServer *server = qobject_cast<QTcpServer*>(sender());
    while(server->hasPendingConnections()) {
        QTcpSocket *local = server->nextPendingConnection();
        QKxFtpClient *client = new QKxFtpClient(local, this);
        QObject::connect(local, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        m_clients.insert(local, client);
    }
}

void QKxFtpServerPrivate::onDisconnected()
{
    if(m_bClose) {
        qDebug() << "onLocalDisconnected";
        return;
    }
    QIODevice *dev = qobject_cast<QIODevice*>(sender());
    QString msg = dev->errorString();
    QKxFtpClient *client = m_clients.take(dev);
    if(client != nullptr) {
        client->deleteLater();
    }
    dev->deleteLater();
}

void QKxFtpServerPrivate::init()
{
    m_server = new QTcpServer(this);
    QObject::connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    if(!m_server->listen(QHostAddress("127.0.0.1"))) {
        qFatal("it's never come here");
    }
    QString addr = QString("tcp://127.0.0.1:%1").arg(m_server->serverPort());
    QByteArray addr2 = addr.toLatin1();
    qputenv(m_envName.toLatin1(), addr2);
}

void QKxFtpServerPrivate::close()
{
    qputenv(m_envName.toLatin1(), "");
    m_bClose = true;
    for(QMap<QPointer<QIODevice>, QPointer<QKxFtpClient> >::Iterator it = m_clients.begin(); it != m_clients.end(); it++) {
        QIODevice *k = it.key();
        QKxFtpClient *v = it.value();
        delete k;
        delete v;
    }
    m_clients.clear();
    if(m_server) {
        m_server->close();
        delete m_server;
    }
    m_worker->quit();
}
