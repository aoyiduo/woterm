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

#ifndef QWOTUNNELSERVER_H
#define QWOTUNNELSERVER_H

#include "qwossh.h"

class QTcpServer;
class QTcpSocket;
// https://api.libssh.org/stable/libssh_tutor_forwarding.html
class QWoTunnelChannel: public QWoSshChannel
{
    Q_OBJECT
public:
    explicit QWoTunnelChannel(QTcpSocket *socket, const QString& sessionName, QObject *parent= nullptr);
    virtual ~QWoTunnelChannel();
signals:
    void dataArrived(const QByteArray& buf);
private slots:
    virtual void onRemoteDataArrived(const QByteArray& buf) = 0;
    virtual void onLocalReadyRead() = 0;
    virtual void onInternalConnectionFinished(bool ok) = 0;
    void onDisconnected();
protected:
    static int mygid(const QString& host, int port);
    virtual void stop();
    void flushAndClose();
    Q_INVOKABLE bool connectToRemote(int gid);
protected:
    QPointer<QTcpSocket> m_socket;
    QString m_sessionName;
};

class QWoTunnelServer : public QObject
{
    Q_OBJECT
public:
    explicit QWoTunnelServer(QObject* parent = nullptr);
    virtual ~QWoTunnelServer();
    virtual bool isPortmapping() = 0;
    bool isRunning();
    void stop();
    static void setMaxSessionMultiplexCount(int cnt = 1000);
    inline QString sessionName() {
        return m_sessionName;
    }
signals:
    void errorArrived(const QString& error);
protected slots:
    void onNewConnection();
    // channel event.
    void onConnectionFinished(bool ok);
    void onFinishArrived(int code);
    void onErrorArrived(const QString& buf, const QVariantMap& userData= QVariantMap());
    void onInputArrived(const QString& title, const QString& prompt, bool visible);
protected:
    virtual QWoTunnelChannel *createChannel(QTcpSocket* client, const QString& sessionName) = 0;
protected:
    bool start(const QString& sessionName, const QString& hostLocal, int portLocal);
protected:
    friend class QWoTunnelChannel;

    QPointer<QTcpServer> m_server;
    QList<QPointer<QWoSshChannel>> m_channels;
    QString m_sessionName;
};


class QWoPortmappingServer : public QWoTunnelServer
{
    Q_OBJECT
public:
    explicit QWoPortmappingServer(QObject *parent = nullptr);
    bool start(const QString& sessionName, const QString& hostLocal, int portLocal, const QString& hostRemote, int portRemote);
    bool isPortmapping() {return true;}
private:
    QWoTunnelChannel *createChannel(QTcpSocket* client, const QString& sessionName);
private:
    QString m_hostRemote;
    int m_portRemote;
};

class QWoSocks45Server : public QWoTunnelServer
{
    Q_OBJECT
public:
    explicit QWoSocks45Server(QObject* parent = nullptr);
    bool start(const QString& sessionName, const QString& hostLocal, int portLocal);
    bool isPortmapping() {return false;}
private:
    QWoTunnelChannel *createChannel(QTcpSocket* client, const QString& sessionName);
};


class QWoTunnelFactory: public QObject
{
    Q_OBJECT
public:
    explicit QWoTunnelFactory(QObject* parent = nullptr);

    static QWoTunnelFactory *instance();

    QWoTunnelServer *find(qint64 id);
    bool isRunning(qint64 id);
    void release(qint64 id);
    void releaseAll();

    template<class T>
    T *get(qint64 id, bool *pbcreated) {
        QWoTunnelServer *server = m_servers.value(id);
        if(pbcreated != nullptr) {
            *pbcreated = false;
        }
        if(server == nullptr) {
            server = new T(this);
            m_servers.insert(id, server);
            if(pbcreated != nullptr) {
                *pbcreated = true;
            }
        }
        return qobject_cast<T*>(server);
    }
protected slots:
    void onAboutToQuit();
private:
    QMap<qint64, QPointer<QWoTunnelServer>> m_servers;
};


#endif // QWOTUNNELSERVER_H
