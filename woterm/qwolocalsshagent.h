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

#ifndef QWOLOCALSSHAGENT_H
#define QWOLOCALSSHAGENT_H

#include <QObject>
#include <QPointer>
#include <QList>

class QTcpServer;
class QTcpSocket;
class QLocalSocket;

class QWoLocalSshAgentClient : public QObject
{
    Q_OBJECT
public:
    explicit QWoLocalSshAgentClient(const QString& sshAuthSock, QTcpSocket *tcp, QObject *parent = nullptr);
    virtual ~QWoLocalSshAgentClient();

private slots:
    void onTcpReadyRead();
    void onLocalReadyRead();

private:
    QPointer<QTcpSocket> m_tcp;
    QPointer<QLocalSocket> m_local;
};

class QWoLocalSshAgentServer : public QObject
{
    Q_OBJECT
public:
    explicit QWoLocalSshAgentServer(QObject *parent = nullptr);
    static QWoLocalSshAgentServer *instance();
    int port() const;
    QString host() const;
private slots:
    void onNewConnection();
    void onClientDestroyed();
private:
    QPointer<QTcpServer> m_server;
    QList<QPointer<QWoLocalSshAgentClient>> m_clients;
};


#endif // QWOLOCALSSHAGENT_H
