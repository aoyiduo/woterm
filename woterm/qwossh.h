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

#pragma once

#include "qwoglobal.h"

#include <QThread>
#include <QMutex>
#include <QPointer>
#include <QMultiMap>
#include <QWaitCondition>

struct MsgRequest;
class QSshClient;
class QSshProxyClient;
class QSshMultClient;
class QWoSshShell;
class QWoSshChannel;

class QWoSSHConnection : public QThread
{
    Q_OBJECT
public:
    explicit QWoSSHConnection(QObject *parent= nullptr);
    virtual ~QWoSSHConnection();
    void append(QWoSshChannel *cli);
    void remove(QWoSshChannel *cli);
    bool hasRunning();
    bool start(const QString& host);
    void stop();
    void setInputResult(const QString& pass);
    void shellWrite(QWoSshChannel *cli, const QByteArray& buf);
    void shellSize(QWoSshChannel *cli, int cols, int rows);
    void sftpOpenDir(QWoSshChannel *cli, const QStringList& paths, const QVariantMap& user);
    void sftpMkDir(QWoSshChannel *cli, const QString& path, int mode, const QVariantMap& user);
    void sftpRmDir(QWoSshChannel *cli, const QString& path, const QVariantMap& user);
    void sftpUnlink(QWoSshChannel *cli, const QString &path, const QVariantMap& user);
    void sftpDownload(QWoSshChannel *cli, const QString& remote, const QString& local, int policy, const QVariantMap& user);
    void sftpUpload(QWoSshChannel *cli, const QString& local, const QString& remote, int policy, const QVariantMap& user);
    void sftpListFile(QWoSshChannel *cli, const QString& path, const QVariantMap& user);
    void sftpAbort(QWoSshChannel *cli);
    void internalUploadNext(QWoSshChannel *cli, const QVariantMap& user);
    void internalListFileNext(QWoSshChannel *cli, const QByteArray& path, const QVariantMap& user);
    void internalRemoveFileNext(QWoSshChannel *cli, const QByteArray& path, bool isDir, const QVariantMap& user);
    void internalAsyncTaskNext(QWoSshChannel *cli, uchar type, const QByteArray& data = QByteArray());
signals:
    void finishArrived(int);
    void errorArrived(const QString& err, const QVariantMap& userData);
    void passwordArrived(const QString& host, const QByteArray& buf);
    void inputArrived(const QString& host, const QString& prompt, bool visble);
    void connectionStart();
    void connectionFinished(bool ok);
private slots:
    void onInputArrived(const QString& host, const QString& prompt, bool visble);
    void onFinished();
    void onConnectionFinished(bool ok);
private:
    bool init(const QString& host);
    void run();
    bool running();
    bool connectToProxy(int i, const QString& host, ushort port);    
private:
    QPointer<QSshMultClient> m_conn;
    QPointer<QSshClient> m_input;
    QList<QPointer<QSshProxyClient>> m_proxys;
    int m_listenSocket;
    ushort m_listenPort;
    int m_connectionState;
};

class QWoSshChannel : public QObject
{
    Q_OBJECT
public:
    explicit QWoSshChannel(QObject *parent=nullptr);
    virtual ~QWoSshChannel();
    bool start(const QString& host, int gid);
    void stop();
    void setInputResult(const QString& pass);
    bool hasRunning();
    QWoSSHConnection *connection();
signals:
    void finishArrived(int);
    void errorArrived(const QString& err, const QVariantMap& userData);
    void passwordArrived(const QString& host, const QByteArray& buf);
    void inputArrived(const QString& host, const QString& prompt, bool visble);
    void connectionStart();
    void connectionFinished(bool ok);
protected:
    virtual bool handleOpen(void *session) = 0;
    virtual void handleClose(int code) = 0;
    virtual bool handleRead() = 0;
    virtual bool handleRequest(struct MsgRequest& msg) = 0;
    virtual void* channel() = 0;
    virtual void init() = 0;
protected:    
    friend class QSshMultClient;
    friend class QWoSSHConnection;
    QPointer<QWoSSHConnection> m_conn;
};

class QWoSshShell : public QWoSshChannel
{
    Q_OBJECT
public:
    explicit QWoSshShell(QObject *parent= nullptr);
    virtual ~QWoSshShell();
    void write(const QByteArray& buf);
    void updateSize(int cols, int rows);
signals:
    void dataArrived(const QByteArray& buf);
protected:
    void emitSendData(const QByteArray& buf);
private:
signals:
    void internalDataArrived(const QByteArray& buf);
private slots:
    void onInternalDataArrived(const QByteArray& buf);
protected:
    void init();
private:
    QAtomicInt m_cacheCount;
};

class QSshFtpClient;
class QWoSshFtp : public QWoSshChannel
{
    Q_OBJECT
public:
    enum TransferPolicy {
        TP_Skip = 0,
        TP_Append = 1,
        TP_Override = 2
    };
public:
    explicit QWoSshFtp(QObject *parent= nullptr);
    virtual ~QWoSshFtp();
    void setLogFile(const QString& logFile);
    void openDir(const QStringList& paths, const QVariantMap& user=QVariantMap());
    void openDir(const QString& path="~", const QVariantMap& user=QVariantMap());
    void mkDir(const QString& path, int mode, const QVariantMap& user=QVariantMap());
    void rmDir(const QString& path, const QVariantMap& user=QVariantMap());
    void unlink(const QString &path, const QVariantMap& user=QVariantMap());
    void download(const QString& remote, const QString& local, TransferPolicy policy = TP_Append, const QVariantMap& user=QVariantMap());
    void upload(const QString& local, const QString& remote, TransferPolicy policy = TP_Append, const QVariantMap& user=QVariantMap());
    void listFile(const QString& path="~", const QVariantMap& user=QVariantMap());
    void abort();
signals:
    void commandStart(int type, const QVariantMap& user);
    void commandFinish(int type, const QVariantMap& user);
    void progress(int type, int v, const QVariantMap& user);
    void dirOpen(const QString& path, const QVariantList& data, const QVariantMap& user);
    // only file result. not any dir.
    // path: the current path.
    // data: the file info under the current path.
    void fileList(const QVariantList& data, const QVariantMap& user);
private:
    virtual void init();
};

class QWoSshFactory : public QObject
{
    Q_OBJECT
public:
    explicit QWoSshFactory(QObject *parent=nullptr);
    virtual ~QWoSshFactory();
    static QWoSshFactory *instance();
    QWoSshShell *createShell(bool cmd = false);
    QWoSshFtp *createSftp();
    void release(QWoSshChannel *obj);
    QWoSSHConnection *get(int gid, bool *pcreated);
private slots:
    void onChannelFinishArrived(int);
    void onConnectionFinishArrived(int);
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QWoSshChannel>> m_dels;
    QMap<int, QPointer<QWoSSHConnection>> m_objs;
};
