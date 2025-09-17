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

struct RLoginMsg;

class QRLoginClient : public QThread
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
    explicit QRLoginClient(const TargetInfo& ti, QObject *parent);
    ~QRLoginClient();
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
    void passwordArrived(const QString& host, const QByteArray& buf);
    void inputArrived(const QString& host, const QString& prompt, bool visible);
private:
    void emitDataArrived(const QByteArray& buf);
signals:
    void internalDataArrived(const QByteArray &buf);
private slots:
    void onFinished();
    void onInternalDataArrived(const QByteArray& buf);
public:
    virtual void handleRead(const QByteArray& buf);
    virtual void handleError(const QString& err);
    virtual bool handleInput(const QString& prompt, QByteArray& result, bool visble);
    virtual void setInputResult(const QString& pass);
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
    QList<RLoginMsg> m_queue;
    int m_msgRead;
    int m_msgWrite;
    int m_codeExit;
    int m_server;
    ushort m_port;
    QAtomicInt m_cacheCount;
};

class QRLoginPtyClient;
class QWoRLogin : public QObject
{
    Q_OBJECT
public:
    explicit QWoRLogin(QObject* parent=nullptr);
    ~QWoRLogin();
    bool hasRunning();
    bool start(const QString& host,int cols,int rows);
    void stop();
    void killAll();
    void setInputResult(const QString& pass);
    void write(const QByteArray& buf);
    void updateSize(int cols, int rows);
    void sendControl(char c);
signals:
    void dataArrived(const QByteArray& buf);
    void finishArrived(int);
    void errorArrived(const QByteArray& buf);
    void passwordArrived(const QString& host, const QByteArray& buf);
    void inputArrived(const QString& host, const QString& prompt, bool visble);
private slots:
    void onInputArrived(const QString& host, const QString& prompt, bool visble);
    void onFinished();
private:
    bool init(const QString& host);
    void cleanup();
private:
    QPointer<QRLoginPtyClient> m_pty;
    QPointer<QRLoginClient> m_input;
    int m_listenSocket;
    ushort m_listenPort;
};

class QWoRLoginFactory : public QObject
{
    Q_OBJECT
public:
    explicit QWoRLoginFactory(QObject *parent=nullptr);
    ~QWoRLoginFactory();
    static QWoRLoginFactory *instance();
    QWoRLogin *create();
    void release(QWoRLogin *obj);
private slots:
    void onFinished(int);
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QWoRLogin>> m_dels;
    QMutex m_mutex;
    QList<RLoginMsg> m_queue;
};
