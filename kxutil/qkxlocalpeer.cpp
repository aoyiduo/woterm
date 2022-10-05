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

#include "qkxlocalpeer.h"

#include <QSettings>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QCoreApplication>
#include <QSharedMemory>
#include <QDataStream>
#include <QPointer>

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

#define TIME_OUT (500)

class QKxLocalPeerPrivate {
    QPointer<QKxLocalPeer> m_p;
    QPointer<QLocalServer> m_server;
    bool m_isClient;
    QString m_name;
public:
    QKxLocalPeerPrivate(const QString& id, bool withSid, QKxLocalPeer *p)
        : m_p(p){
#ifdef Q_OS_WIN
        DWORD sid;
        ProcessIdToSessionId(GetCurrentProcessId(), &sid);
#else
        qint32 sid = ::getuid();
#endif
        m_name = withSid ? QString("localpeer:%1-%2").arg(id).arg(sid) : QString("localpeer:%1").arg(id);
        m_isClient = initClient();
    }

    ~QKxLocalPeerPrivate() {
        if(m_server) {
            m_server->close();
            m_server->deleteLater();
        }
    }

    bool initClient() {
        QLocalSocket socket;
        socket.connectToServer(m_name);
        if(socket.waitForConnected(TIME_OUT)) {
            return true;
        }
        m_server = new QLocalServer(m_p);
        m_server->setSocketOptions(QLocalServer::WorldAccessOption);
        QObject::connect(m_server, SIGNAL(newConnection()), m_p, SLOT(onNewConnection()));
        if(!m_server->listen(m_name)) {
            if(m_server->serverError() == QAbstractSocket::AddressInUseError) {
                QLocalServer::removeServer(m_name);
                if(!m_server->listen(m_name)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool isClient() {
        return m_isClient;
    }

    bool sendMessage(const QString &msg, int timeout) {
        QLocalSocket local;
        local.connectToServer(m_name);
        if(!local.waitForConnected(timeout)) {
            return false;
        }
        local.write(msg.toLatin1());
        local.flush();
        local.waitForReadyRead(1000);
        return true;
    }

    void onNewConnection() {
        while(m_server->hasPendingConnections()) {
            QLocalSocket *local = m_server->nextPendingConnection();
            QObject::connect(local, SIGNAL(readyRead()), m_p, SLOT(onReadReady()));
            QObject::connect(local, SIGNAL(aboutToClose()), local, SLOT(deleteLater()));
        }
    }
};

QKxLocalPeer::QKxLocalPeer(const QString &id, bool withSid, QObject *parent)
    : QObject(parent)
{
    m_prv = new QKxLocalPeerPrivate(id, withSid, this);
}

QKxLocalPeer::QKxLocalPeer(const QString &id, QObject *parent)
    : QKxLocalPeer(id, true, parent)
{

}

QKxLocalPeer::~QKxLocalPeer()
{
    delete m_prv;
}

bool QKxLocalPeer::isClient()
{
    return m_prv->isClient();
}

bool QKxLocalPeer::sendMessage(const QString &msg, int timeout)
{
    return m_prv->sendMessage(msg, timeout);
}

void QKxLocalPeer::onNewConnection()
{
    return m_prv->onNewConnection();
}

void QKxLocalPeer::onReadReady()
{
    QLocalSocket *local = qobject_cast<QLocalSocket*>(sender());
    QByteArray buf = local->readAll();
    emit messageReceived(buf);
    local->write("ok");
    local->flush();
    local->close();
    local->deleteLater();
    qDebug() << "onReadyReady" << buf;
}
