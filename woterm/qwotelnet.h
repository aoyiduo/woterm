/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#pragma once

#include "qwoglobal.h"

#include <QThread>
#include <QPointer>
#include <QMutex>
#include <QAtomicInt>

struct TelnetMsg;

class QTelnetClient : public QThread
{
    Q_OBJECT
public:
    struct TargetInfo {
        QByteArray name;
        QByteArray host;
        ushort port;
        QByteArray user;
        QByteArray password;
        TargetInfo(const HostInfo& hi) {
            name = hi.name.toUtf8();
            host = hi.host.toUtf8();
            port = ushort(hi.port);
            user = hi.user.toUtf8();
            password = hi.password.toUtf8();
        }
    };
public:
    explicit QTelnetClient(const TargetInfo& ti, QObject *parent);
    ~QTelnetClient();
    inline int exitCode(){
        return m_codeExit;
    }
    inline QString host() const {
        return m_ti.host;
    }
    inline ushort port() const {
        return ushort(m_ti.port);
    }
    bool init(int server, ushort port);
    void stop();
signals:
    void dataArrived(const QByteArray& buf);    
    void errorArrived(const QByteArray& buf);
private:
    void emitDataArrived(const QByteArray& buf);
signals:
    void internalDataArrived(const QByteArray& buf);
private slots:
    void onFinished();
    void onInternalDataArrived(const QByteArray& buf);
public:
    virtual void handleRead(const QByteArray& buf);
    virtual void handleError(const QString& err);
protected:
    virtual void run();
    virtual int running();
protected:
    void push(uchar type, const QByteArray& data = QByteArray());
    bool pop(uchar &type, QByteArray& data);
    bool takeOne(uchar type, QByteArray& data);

protected:
    TargetInfo m_ti;
    QMutex m_mutex;
    QList<TelnetMsg> m_queue;
    int m_msgRead;
    int m_msgWrite;
    int m_codeExit;
    int m_server;
    ushort m_port;
    QAtomicInt m_cacheCount;
};

class QTelnetPtyClient;
class QWoTelnet : public QObject
{
    Q_OBJECT
public:
    explicit QWoTelnet(QObject* parent=nullptr);
    ~QWoTelnet();
    bool hasRunning();
    bool start(const QString& host);
    void stop();
    void killAll();
    void write(const QByteArray& buf);
    void updateSize(int cols, int rows);
    void sendControl(char c);
signals:
    void dataArrived(const QByteArray& buf);
    void finishArrived(int);
    void errorArrived(const QByteArray& buf);
private slots:
    void onFinished();
private:
    bool init(const QString& host);
    void cleanup();
private:
    QPointer<QTelnetPtyClient> m_pty;
    QPointer<QTelnetClient> m_input;
    int m_listenSocket;
    ushort m_listenPort;
};

class QWoTelnetFactory : public QObject
{
    Q_OBJECT
public:
    explicit QWoTelnetFactory(QObject *parent=nullptr);
    ~QWoTelnetFactory();
    static QWoTelnetFactory *instance();
    QWoTelnet *create();
    void release(QWoTelnet *obj);
private slots:
    void onFinished(int);
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QWoTelnet>> m_dels;
    QMutex m_mutex;
    QList<TelnetMsg> m_queue;
};
