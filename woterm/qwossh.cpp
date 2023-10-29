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

#include "qwossh.h"
#include "qwosshconf.h"
#include "qkxver.h"
#include "qwoutils.h"
#include "qwosetting.h"
#include "qwoidentify.h"
#include "qwolocalsshagent.h"

#include <qssh.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/sftp.h>

#include <QEventLoop>
#include <QDebug>
#include <QDataStream>
#include <QWaitCondition>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QBuffer>
#include <QAtomicInt>
#include <QUrl>
#include <QMap>
#include <QAtomicInt>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fcntl.h>
#define myclosesocket  closesocket

#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#define myclosesocket(x)    close(x)
#endif

#define MAX_CHANNEL  (100)
#define MAX_BUFFER   (1024*70)

#define FTP_TYPE_OPENDIR    (1)

static ulong LOCAL_IP = inet_addr("127.0.0.1");


static int connectToAgent(const QByteArray& host, int port) {
    struct addrinfo *ai = nullptr;
    struct addrinfo *itr = nullptr;

    int err = QWoUtils::getAddrInfos(host, port, &ai);
    if(err != 0) {
        return 0;
    }
    int sockFd = -1;
    for (itr = ai; itr != nullptr; itr = itr->ai_next) {
        /* create socket */
        socket_t fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);
        err = ::connect(fd, itr->ai_addr, itr->ai_addrlen);
        if(err != 0){
            myclosesocket(fd);
            continue;
        }
        sockFd = fd;
        break;
    }
    QWoUtils::freeAddrInfos(ai);
    return sockFd;
}

static int handleAuthenticateByInternalKeys(ssh_session session, int method) {
    if(method & SSH_AUTH_METHOD_PUBLICKEY) {
        // inner
        QMap<QString, IdentifyInfo> all = QWoIdentify::loadFromSqlite();
        for(auto it = all.begin(); it != all.end(); it++) {
            QString name = it.key();
            IdentifyInfo ti = it.value();
            QByteArray prvKey = ti.prvKey;
            ssh_key key = nullptr;
            int err = ssh_pki_import_privkey_base64(prvKey.data(), nullptr, nullptr, nullptr, &key);
            if(err == SSH_OK) {
                err = ssh_userauth_publickey(session, nullptr, key);
                ssh_key_free(key);
                if(err == SSH_AUTH_SUCCESS) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int handleAuthenticateByDotSshPath(ssh_session session, int method) {
    if(method & SSH_AUTH_METHOD_PUBLICKEY) {
        if(QKxVer::instance()->isFullFeather() && QWoSetting::enableUserHomeIdentityFiles()) {
            QString path = QDir::cleanPath(QDir::homePath() + "/.ssh");
            QDir d(path);
            QFileInfoList fis = d.entryInfoList(QDir::Files);
            for(auto it = fis.begin(); it != fis.end(); it++) {
                QFileInfo fi = *it;
                QString fileName = fi.fileName();
                if(fileName.endsWith(".pub")) {
                    continue;
                }
                QFile f(fi.absoluteFilePath());
                if(!f.open(QFile::ReadOnly)) {
                    continue;
                }
                QByteArray prvKey = f.readAll();
                f.close();
                ssh_key key = nullptr;
                int err = ssh_pki_import_privkey_base64(prvKey.data(), nullptr, nullptr, nullptr, &key);
                if(err == SSH_OK) {
                    err = ssh_userauth_publickey(session, nullptr, key);
                    ssh_key_free(key);
                    if(err == SSH_AUTH_SUCCESS) {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

static int handleAuthenticateBySshAgent(ssh_session session, int method) {
    if(method & SSH_AUTH_METHOD_PUBLICKEY) {
        if(QKxVer::instance()->isFullFeather() && QWoSetting::enableLocalSshAgent()) {
            QString addr = QWoSetting::lastLocalSshAgentAddress();
            if(addr.isEmpty()) {
                addr = QWoSetting::sshAuthSockDefault();
            }
            if(addr[0] == "/") {
                ssh_options_set(session, SSH_OPTIONS_IDENTITY_AGENT, addr.toLatin1());
                int err = ssh_userauth_agent(session, nullptr);
                if(err == SSH_AUTH_SUCCESS) {
                    return 1;
                }
            }else{
                QString host = QWoLocalSshAgentServer::instance()->host();
                int port = QWoLocalSshAgentServer::instance()->port();
                int fd = connectToAgent(host.toUtf8(), port);
                if(fd > 0) {
                    ssh_set_agent_socket(session, fd);
                    int err = ssh_userauth_agent(session, nullptr);
                    myclosesocket(fd);
                    if(err == SSH_AUTH_SUCCESS) {
                        return 1;
                    }
                }
            }
        }

        if(QKxVer::instance()->isFullFeather() && QWoSetting::enableRemoteSshAgent()) {
            QString addr = QWoSetting::remoteSshAgentAddress();
            QStringList hp = addr.split(':');
            if(hp.size() != 2) {
                return 0;
            }

            QByteArray host = hp.at(0).toUtf8();
            int port = hp.at(1).toInt();
            int fd = connectToAgent(host, port);
            if(fd < 0) {
                return 0;
            }
            ssh_set_agent_socket(session, fd);
            int err = ssh_userauth_agent(session, nullptr);
            myclosesocket(fd);
            if(err == SSH_AUTH_SUCCESS) {
                return 1;
            }
        }
    }

    return 0;
}

class QSshInteractiveClient : public QSshAuthClient
{
public:
    explicit QSshInteractiveClient(const TargetInfo& ti, QObject *parent)
        : QSshAuthClient(ti, parent){

    }

    virtual bool open() = 0;
private:
    int authenticateThirdparty(ssh_session session, int method) {
        int err = 0;
        err = handleAuthenticateByInternalKeys(session, method);
        if(err != 0) {
            return err;
        }
        if(!QKxVer::instance()->isFullFeather() || !QWoSetting::allowToUseExternalIdentityFiles()) {
            return 0;
        }
        err = handleAuthenticateByDotSshPath(session, method);
        if(err != 0) {
            return err;
        }
        err = handleAuthenticateBySshAgent(session, method);
        if(err != 0) {
            return err;
        }
        return 0;
    }
};

class QPowerProxyClient : public QSshProxyClient {
public:
    explicit QPowerProxyClient(const TargetInfo& ti, QObject* parent)
        : QSshProxyClient(ti, parent){

    }

private:
    int authenticateThirdparty(ssh_session session, int method) {
        int err = 0;
        err = handleAuthenticateByInternalKeys(session, method);
        if(err != 0) {
            return err;
        }
        if(!QKxVer::instance()->isFullFeather() || !QWoSetting::allowToUseExternalIdentityFiles()) {
            return 0;
        }
        err = handleAuthenticateByDotSshPath(session, method);
        if(err != 0) {
            return err;
        }
        err = handleAuthenticateBySshAgent(session, method);
        if(err != 0) {
            return err;
        }
        return 0;
    }
};

class QWoPowerShell : public QWoSshShell
{
private:
    ssh_channel m_channel;
    int m_colsLast, m_rowsLast;
    bool m_cmd;
public:
    QWoPowerShell(bool cmd, QObject *parent)
        : QWoSshShell(parent)
        , m_channel(nullptr)
        , m_colsLast(80)
        , m_rowsLast(24)
        , m_cmd(cmd) {

    }

    virtual ~QWoPowerShell() {
        if(m_channel) {
            ssh_channel_close(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
    }

    void* channel() {
        return m_channel;
    }
public:
    bool _handleOpen(void *session) {
        m_channel = ssh_channel_new(ssh_session(session));
        if (m_channel == nullptr){
            return false;
        }
        ssh_channel_set_blocking(m_channel, 1);
        if (ssh_channel_open_session(m_channel) != SSH_OK) {
            return false;
        }
        if(ssh_channel_request_pty_size(m_channel, "xterm", m_colsLast, m_rowsLast) != SSH_OK) {
            return false;
        }
        if(!m_cmd){
            if(ssh_channel_request_shell(m_channel) != SSH_OK) {
                return false;
            }
        }
        return true;
    }

    bool handleOpen(void *session) {
        bool ok = _handleOpen(session);
        emit connectionFinished(ok);
        return ok;
    }

    void handleClose(int code) {
       // qDebug() << "handleClose" << "ssh_channel_get_exit_status" << m_lastCommandExitStatus;
        if(m_channel) {
            ssh_channel_close(m_channel);
            m_lastCommandExitStatus = ssh_channel_get_exit_status(m_channel);
            ssh_channel_free(m_channel);
            m_channel = nullptr;
        }
        emit finishArrived(code);
    }

    bool handleRead() {        
        int total = 0;
        while(1){
            QByteArray buf(MAX_BUFFER, Qt::Uninitialized);
            int nbytes = ssh_channel_read_nonblocking(m_channel, buf.data(), MAX_BUFFER, 0);
            if(nbytes == 0) {
                return true;
            }else if(nbytes < 0) {                
                return false;
            }
            total += nbytes;
            buf.resize(nbytes);
            emitSendData(buf);
        }
        return true;
    }

    bool handleRequest(MsgRequest &msg) {
        if(m_channel == nullptr) {
            return false;
        }
        if(msg.type == MT_PTYDATA) {
            if(m_cmd) {
                int err = ssh_channel_request_exec(m_channel, msg.data);
                return err != SSH_ERROR;
            }
            int err = ssh_channel_write(m_channel, msg.data.data(), msg.data.length());
            return err != SSH_ERROR;
        }else if(msg.type == MT_PTYSIZE) {
            int cols, rows;
            QDataStream buf(msg.data);
            buf >> cols >> rows;
            m_colsLast = cols;
            m_rowsLast = rows;
            int err = ssh_channel_change_pty_size(m_channel, cols, rows);
            return err != SSH_ERROR;
        }else if(msg.type == MT_SIGNAL) {
            QByteArray sig;
            QDataStream buf(msg.data);
            buf >> sig;
            int err = ssh_channel_request_send_signal(m_channel, sig);
            return err != SSH_ERROR;
        }else if(msg.type == MT_BREAK) {
            int length;
            QDataStream buf(msg.data);
            buf >> length;
            int err = ssh_channel_request_send_break(m_channel, length);
            return err != SSH_ERROR;
        }
        return true;
    }

    bool handleSize(int cols, int rows) {
        ssh_channel_change_pty_size(m_channel, cols, rows);
        return true;
    }

    bool handleWrite(const QByteArray& data) {
        int err = ssh_channel_write(m_channel, data.data(), data.length());
        return err == SSH_ERROR;
    }
};

class QWoPowerSftp : public QWoSshFtp
{
    ssh_channel m_channel;
    sftp_session m_sftp;

    enum EAsynOperation { eNone, eUpload, eDownload, eRmDir, eListFile, eChmodFile};
    EAsynOperation m_opAsync;
    QList<MsgRequest> m_asyncTasks;

    QVariantMap m_userData;

    QFile m_lfile;
    sftp_file m_rfile;
    qint64 m_total;
    int m_asyncRequest, m_percent, m_tickLast, m_speedPerSecond;
    QByteArray m_pathCurrent;
    bool m_abort;
    // for list file next.
    QList<QByteArray> m_listFileNext;

    // for chmod folder next.
    struct ChmodFolderNext {
        QByteArray path; // path for folder.
        int permission;
    };

    QList<ChmodFolderNext> m_chmodFolderNext;
    // for rmDir
    QList<QVariantMap> m_rmDirNext;
    QString m_errorString;
public:
    explicit QWoPowerSftp(QObject *parent = nullptr)
     : QWoSshFtp(parent)
     , m_channel(nullptr)
     , m_sftp(nullptr)
     , m_opAsync(eNone)
     , m_rfile(nullptr)
     , m_abort(false) {

    }
    virtual ~QWoPowerSftp() {
        if(m_rfile) {
            sftp_close(m_rfile);
        }
        if(m_sftp) {
            sftp_free(m_sftp);
        }
    }

    bool uploadNext(const QVariantMap& user) {
        if(m_conn) {
            m_conn->internalUploadNext(this, user);
        }
        return true;
    }

    bool listFileNext(const QVariantMap& user) {
        if(m_listFileNext.isEmpty()) {
            return false;
        }
        if(m_conn) {
            QByteArray path = m_listFileNext.takeFirst();
            m_conn->internalListFileNext(this, path, user);
            return true;
        }
        return false;
    }

    bool chmodFolderNext(const QVariantMap& user) {
        if(m_chmodFolderNext.isEmpty()) {
            return false;
        }
        if(m_conn) {
            ChmodFolderNext next = m_chmodFolderNext.takeFirst();
            m_conn->internalChmodFolderNext(this, next.path, next.permission, user);
            return true;
        }
        return false;
    }

    bool rmDirNext(const QVariantMap& user) {
        if(m_rmDirNext.isEmpty()) {
            return false;
        }
        if(m_conn) {
            QVariantMap next = m_rmDirNext.takeLast();
            QByteArray path = next.value("filePath").toByteArray();
            bool isDir = next.value("isDir").toBool();
            m_conn->internalRemoveFileNext(this, path, isDir, user);
            return true;
        }
        return false;
    }

    void runAyncTaskNext() {
        if(!m_asyncTasks.isEmpty()) {
            const MsgRequest& req = m_asyncTasks.takeFirst();
            m_conn->internalAsyncTaskNext(this, req.type, req.data);
        }
    }

private:
    inline QByteArray canonicalizePath(const QByteArray& _path) {
        QByteArray path = _path;
        if(path.startsWith('~')) {
            char *tmp = sftp_canonicalize_path(m_sftp, ".");
            if(tmp == nullptr) {
                return QByteArray();
            }
            path = path.mid(1);
            path.insert(0, tmp);
            ssh_string_free_char(tmp);
        }
        return path;
    }

    void handleError(const QString& err, const QVariantMap& userData) {
        m_errorString = err;
        emit errorArrived(err, userData);
    }

    void handleCommandStart(int type, const QVariantMap& user) {
        emit commandStart(type, user);
    }

    void handleCommandFinish(int type, const QVariantMap& user) {
        emit commandFinish(type, user);
        runAyncTaskNext();
    }

    QVariantMap mkDirResult(int err, const QVariantMap& user) {
        QVariantMap dm = user;
        if(err == -999) {
            dm.insert("reason", "abort");
        }else if(err < 0) {
            int code = sftpLastError(dm);
            if(code == SSH_FX_FILE_ALREADY_EXISTS) {
                dm.insert("reason", "ok");
            }else if(code == 0 || code == SSH_FX_NO_CONNECTION || code == SSH_FX_CONNECTION_LOST) {
                // net broken, will has code == 0. so add it.
                dm.insert("reason", "fatal");
            }else{
                dm.insert("reason", "error");
            }
        }else{
            dm.insert("reason", "ok");
        }
        return dm;
    }

    bool channelValid() {
        ssh_channel rchn = ssh_channel(m_sftp->channel);
        return !(ssh_channel_is_eof(rchn) || ssh_channel_is_closed(rchn));
    }

    QVariantMap reasonResult(int err, const QVariantMap& user) {
        QVariantMap dm = user;
        if(err == -999) {
            dm.insert("code", -999);
            dm.insert("reason", "abort");
        }else if(err < 0) {
            int code = sftpLastError(dm);
            dm.insert("code", code);
            qDebug() << "reasonResult" << err << code << user;
            if(code == 0) {
                if(channelValid()) {
                    dm.insert("reason", "error");
                }else{
                    dm.insert("reason", "fatal");
                }
            }else if(code == SSH_FX_NO_CONNECTION || code == SSH_FX_CONNECTION_LOST) {
                // net broken, will has code == 0. so add it.                
                dm.insert("reason", "fatal");
            }else{
                dm.insert("reason", "error");
            }
        }else{
            dm.insert("code", 0);
            dm.insert("reason", "ok");
        }
        return dm;
    }

    /**
     * It cannot be fully trusted. In some cases, it returns an error code of 0 even though the network is disconnected
     */
    bool isFtpFatal() {
        int code = sftp_get_error(m_sftp);
        if(code == SSH_FX_NO_CONNECTION || code == SSH_FX_CONNECTION_LOST) {
            return true;
        }
        return false;
    }

    int sftpLastError(QVariantMap& dm) {
        int code = sftp_get_error(m_sftp);
        switch (code) {
        case SSH_FX_FILE_ALREADY_EXISTS:
            dm.insert("errorString", tr("The file is already exist"));
            break;
        case SSH_FX_NO_SUCH_FILE:
        case SSH_FX_NO_SUCH_PATH:
            dm.insert("errorString", tr("No such file or directory path exists"));
            break;
        case SSH_FX_PERMISSION_DENIED:
            dm.insert("errorString", tr("Permission denied"));
            break;
        case SSH_FX_FAILURE:
            dm.insert("errorString", tr("Generic failure"));
            break;
        case SSH_FX_BAD_MESSAGE:
            dm.insert("errorString", tr("Garbage received from server"));
            break;
        case SSH_FX_NO_CONNECTION:
            dm.insert("errorString", tr("No connection has been set up"));
            break;
        case SSH_FX_CONNECTION_LOST:
            dm.insert("errorString", tr("There was a connection, but we lost it"));
            break;
        case SSH_FX_OP_UNSUPPORTED:
            dm.insert("errorString", tr("Operation not supported by the server"));
            break;
        case SSH_FX_INVALID_HANDLE:
            dm.insert("errorString", tr("Invalid file handle"));
            break;
        case SSH_FX_WRITE_PROTECT:
            dm.insert("errorString", tr("We are trying to write on a write-protected filesystem"));
            break;
        case SSH_FX_NO_MEDIA:
            dm.insert("errorString", tr("No media in remote drive"));
            break;
        }
        return code;
    }

    int runListFile(const QByteArray& path, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_LISTFILE, user);
        m_listFileNext.clear();
        m_opAsync = eListFile;
        int code = _runListFile(path, m_userData);
        if(code <= 0) {
            m_opAsync = eNone;
            m_listFileNext.clear();
            handleCommandFinish(MT_FTP_LISTFILE, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runListFile(const QByteArray& _path, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        sftp_dir dir = sftp_opendir(m_sftp, path);
        if(dir == nullptr) {
            return -2;
        }
        QByteArray basePath = user.value("basePath").toByteArray();
        sftp_attributes attr;
        QList<QVariant> files;
        while ((attr = sftp_readdir(m_sftp, dir)) != nullptr && !m_abort){
            QByteArray name = attr->name;
            if(name == "." || name == "..") {
                continue;
            }
            if(attr->type == 2) {
                m_listFileNext.append(path + "/" + name);
            }else{
                QVariantMap v;
                QByteArray filePath = path + "/" + name;
                filePath = filePath.replace(0, basePath.length(), "./");
                v.insert("filePath", filePath);
                v.insert("fileSize", qint64(attr->size));
                files.append(v);
            }
            sftp_attributes_free(attr);
        }
        emit fileList(files, user);
        if (sftp_closedir(dir) != SSH_NO_ERROR) {
            return -3;
        }
        if(m_abort) {
            return -999;
        }
        return listFileNext(user) ? 1 : 0;
    }

    int runOpenDir(const QStringList &paths, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_OPENDIR, user);
        int code = 0;
        for(int i = 0; i < paths.length(); i++) {
            QString path = paths.at(i);
            QVariantMap dm = user;
            dm.insert("path", path);
            code = _runOpenDir(path.toUtf8(), dm);
            if(code == 0) {
                break;
            }
        }
        handleCommandFinish(MT_FTP_OPENDIR, reasonResult(code, m_userData));
        return code;
    }

    int _runOpenDir(const QByteArray& _path, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        sftp_dir dir = sftp_opendir(m_sftp, path);
        if(dir == nullptr) {
            return -2;
        }
        sftp_attributes attr;
        QList<QList<QByteArray>> all;
        int ownerMax = 0, groupMax = 0, sizeMax = 0, dateMax = 0, nameMax = 0;
        while ((attr = sftp_readdir(m_sftp, dir)) != nullptr && !m_abort){
            QByteArray longName(attr->longname);
            QByteArray type = filePermissionToText(attr->type, attr->permissions);
            QByteArray owner(attr->owner);
            QByteArray group(attr->group);
            QByteArray size(QByteArray::number(qint64(attr->size)));
            qint64 mtime = qMax(qint64(attr->mtime), qint64(attr->mtime64));
            if(mtime < 10) {
                mtime = qMax(qint64(attr->atime), qint64(attr->atime64));
            }
            QByteArray date = QByteArray::number(mtime);
            //QDateTime dt = QDateTime::fromMSecsSinceEpoch(mtime * 1000);
            //QByteArray date = dt.toString("dd.MM.yyyy hh:mm:ss").toLatin1();
            QByteArray name(attr->name);
            if(ownerMax < owner.length()) {
                ownerMax = owner.length();
            }
            if(groupMax < group.length()) {
                groupMax = group.length();
            }
            if(sizeMax < size.length()) {
                sizeMax = size.length();
            }
            if(dateMax < date.length()) {
                dateMax = date.length();
            }
            if(nameMax < name.length()) {
                nameMax = name.length();
            }
            QList<QByteArray> item;
            item.append(longName);
            item.append(type);
            item.append(owner);
            item.append(group);
            item.append(size);
            item.append(date);
            item.append(name);
            all.append(item);
            sftp_attributes_free(attr);
        }
        QList<QVariant> vall;
        for(int i = 0; i < all.length(); i++) {
            const QList<QByteArray> &item = all.at(i);
            QVariantMap mdata;
            QByteArray longName = item.at(0);
            mdata.insert("longName", item.at(0));
            QString label = item.at(1);
            mdata.insert("type", QString("%1").arg(label.at(0)));
            mdata.insert("permission", label);
            {
                QByteArray owner = item.at(2);
                mdata.insert("owner", owner);
                int nleft = (ownerMax + 7) / 8 * 8 + 1 - owner.length();
                for(int i = 0; i < nleft; i++) {
                    owner.prepend(QChar::Space);
                }
                label.append(owner);
            }
            {
                QByteArray group = item.at(3);
                mdata.insert("group", group);
                int nleft = (groupMax + 7) / 8 * 8 + 1 - group.length();
                for(int i = 0; i < nleft; i++) {
                    group.prepend(QChar::Space);
                }
                label.append(group);
            }
            {
                QByteArray size = item.at(4);
                mdata.insert("size", size);
                int nleft = (sizeMax + 7) / 8 * 8 + 1 - size.length();
                for(int i = 0; i < nleft; i++) {
                    size.prepend(QChar::Space);
                }
                label.append(size);
            }
            {
                QByteArray date = item.at(5);
                mdata.insert("date", date);
                int nleft = (dateMax + 7) / 8 * 8 + 1 - date.length();
                for(int i = 0; i < nleft; i++) {
                    date.prepend(QChar::Space);
                }
                label.append(date);
            }
            {
                QByteArray name = item.at(6);
                mdata.insert("name", name);
                int nleft = (nameMax + 7) / 8 * 8 + 1 - name.length();
                for(int i = 0; i < nleft; i++) {
                    name.prepend(QChar::Space);
                }
                label.append(name);
            }
            mdata.insert("label", label);
            vall.append(mdata);
        }
        if (sftp_closedir(dir) != SSH_NO_ERROR) {
            return -3;
        }
        char *tmp = sftp_canonicalize_path(m_sftp, path);
        if(tmp == nullptr) {
            return -4;
        }
        QVariantMap dm = user;
        dm.insert("absPath", QByteArray(tmp));
        emit dirOpen(QByteArray(tmp), vall, dm);
        ssh_string_free_char(tmp);
        m_pathCurrent = tmp;
        return 0;
    }

    int runFileContent(const QByteArray& remote, qint64 offset, qint64 maxSize, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_READ_FILE_CONTENT, user);
        int code = _runFileContent(remote, offset, maxSize, m_userData);
        if(code <= 0) {
            handleCommandFinish(MT_FTP_READ_FILE_CONTENT, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runFileContent(const QByteArray& _path, qint64 offset, qint64 maxSize, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        sftp_attributes attr = sftp_stat(m_sftp, path);
        if(attr == nullptr) {
            return -2;
        }
        qint64 total = attr->size;
        sftp_attributes_free(attr);
        sftp_file rf = sftp_open(m_sftp, path, O_RDONLY, 0);
        if(rf == nullptr) {
            return -4;
        }
        if(offset > total || offset < 0) {
            sftp_close(rf);
            return -5;
        }
        if(sftp_seek64(rf, offset) < 0) {
            sftp_close(rf);
            return -8;
        }
        QByteArray buf;
        buf.resize(maxSize);
        int n = sftp_read(rf, buf.data(), maxSize);
        if(n > 0) {
            buf.resize(n);
        }
        sftp_close(rf);
        QVariantMap file;
        file.insert("content", buf);
        file.insert("fileSize", total);
        file.insert("offset", offset);
        user.insert("fileContent", file);
        return 0;
    }

    int runWriteFileContent(const QByteArray& remote, const QByteArray& content, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_WRITE_FILE_CONTENT, user);
        int code = _runWriteFileContent(remote, content, m_userData);
        if(code <= 0) {
            handleCommandFinish(MT_FTP_WRITE_FILE_CONTENT, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runWriteFileContent(const QByteArray& _path, const QByteArray& content, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        int access_type = O_WRONLY | O_CREAT | O_TRUNC;
        sftp_file wf = sftp_open(m_sftp, path, access_type, 0600);
        if(wf == nullptr) {
            return -1;
        }
        if(sftp_write(wf, content.data(), content.length()) != content.length()) {
            sftp_close(wf);
            return -2;
        }
        sftp_close(wf);
        return 0;
    }

    int runDownload(const QByteArray& remote, const QByteArray& local, int policy, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_DOWNLOAD, user);
        emit progress(MT_FTP_DOWNLOAD, 0, user);
        m_opAsync = eDownload;
        int code = _runDownload(remote, local, policy, m_userData);
        if(code <= 0) {
            m_opAsync = eNone;
            handleCommandFinish(MT_FTP_DOWNLOAD, reasonResult(code, user));
        }
        return code;
    }

    int _runDownload(const QByteArray& _remote, const QByteArray& local, int policy, QVariantMap& user) {
        QByteArray remote = canonicalizePath(_remote);
        if(remote.isEmpty()) {
            return -1;
        }
        sftp_attributes attr = sftp_stat(m_sftp, remote);
        if(attr == nullptr) {
            return -2;
        }
        qint64 total = attr->size;
        sftp_attributes_free(attr);
        sftp_file rf = sftp_open(m_sftp, remote, O_RDONLY, 0);
        if(rf == nullptr) {
            return -4;
        }
        QString filePath = QDir::cleanPath(QString::fromUtf8(local));
        QString path = filePath.left(filePath.lastIndexOf('/'));
        QDir d;
        d.mkpath(path);
        m_lfile.setFileName(filePath);
        qint64 seekOffset = 0;
        if(m_lfile.exists()) {
            if(policy == QWoSshFtp::TP_Override){
                m_lfile.remove();
                if(!m_lfile.open(QFile::WriteOnly)) {
                    QString errString = m_lfile.errorString();
                    m_userData.insert("errorString", errString);
                    sftp_close(rf);
                    return -5;
                }
            }else{
                if(!m_lfile.open(QFile::Append)) {
                    QString errString = m_lfile.errorString();
                    m_userData.insert("errorString", errString);
                    sftp_close(rf);
                    return -6;
                }
                seekOffset = m_lfile.size();
            }
        }else{
            if(!m_lfile.open(QFile::WriteOnly)) {
                QString errString = m_lfile.errorString();
                QFile::FileError err = m_lfile.error();
                m_userData.insert("errorString", errString);
                qDebug() << "Local file open:" << errString << err;
                sftp_close(rf);
                return -7;
            }
        }

        if(m_lfile.size() == total) {
            sftp_close(rf);
            m_lfile.close();
            return 0;
        }
        if(sftp_seek64(rf, seekOffset) < 0) {
            sftp_close(rf);
            m_lfile.close();
            return -8;
        }

        m_rfile = rf;
        m_percent = 0;
        m_speedPerSecond = 0;
        m_tickLast = QDateTime::currentSecsSinceEpoch();
        m_total = total;
        sftp_file_set_nonblocking(rf);
        m_asyncRequest = sftp_async_read_begin(rf, MAX_BUFFER);
        return 1;
    }

    int _runDownloading() {
        if(m_abort) {
            return -999;
        }
        if(m_rfile == nullptr) {
            return -1;
        }
        QByteArray buf(MAX_BUFFER, Qt::Uninitialized);
        int n = sftp_async_read(m_rfile, buf.data(), buf.length(), m_asyncRequest);
        if(n <= 0){
            return n;
        }
        m_lfile.write(buf, n);
        m_speedPerSecond += n;
        m_asyncRequest = sftp_async_read_begin(m_rfile, MAX_BUFFER);
        if(m_total > 0) {
            int now = QDateTime::currentSecsSinceEpoch();
            int v = m_lfile.size() * 100 / m_total;
            if(now != m_tickLast) {
                m_percent = v;
                m_userData.insert("speed", m_speedPerSecond);
                m_speedPerSecond = 0;
                m_tickLast = now;
                emit progress(MT_FTP_DOWNLOAD, v, m_userData);
            }else if(v != m_percent) {
                m_percent = v;
                emit progress(MT_FTP_DOWNLOAD, v, m_userData);
            }
        }else{
            emit progress(MT_FTP_DOWNLOAD, 0, m_userData);
        }
        return n;
    }


    int runUpload(const QByteArray& local, const QByteArray& remote, int policy, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_UPLOAD, user);
        emit progress(MT_FTP_UPLOAD, 0, user);
        m_opAsync = eUpload;
        int code = _runUpload(local, remote, policy, m_userData);
        if(code <= 0) {
            m_opAsync = eNone;
            handleCommandFinish(MT_FTP_UPLOAD, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runUpload(const QByteArray& local, const QByteArray& _remote, int policy, QVariantMap& user) {
        QByteArray remote = canonicalizePath(_remote);
        if(remote.isEmpty()) {
            return -1;
        }
        int access_type = O_WRONLY | O_CREAT | O_TRUNC;
        qint64 fileOffset = 0;
        sftp_attributes attr = sftp_stat(m_sftp, remote);
        if(attr == nullptr) {
            // file not exist.
            QString path = QDir::cleanPath(remote);
            path = path.left(path.lastIndexOf('/'));
            sftp_mkdir(m_sftp, path.toUtf8(), 0x1FF);
        }else{
            fileOffset = attr->size;
            sftp_attributes_free(attr);
            if(policy == QWoSshFtp::TP_Override) {
                fileOffset = 0;
                access_type = O_WRONLY | O_CREAT | O_TRUNC;
            }else{
                access_type = O_WRONLY|O_APPEND;
            }
        }
        sftp_file wf = sftp_open(m_sftp, remote, access_type, 0600);
        if(wf == nullptr) {
            return -1;
        }
        if(m_lfile.isOpen()) {
            m_lfile.close();
        }
        m_lfile.setFileName(local);
        if(!m_lfile.open(QFile::ReadOnly)) {
            QString errString = m_lfile.errorString();
            m_userData.insert("errorString", errString);
            sftp_close(wf);
            return -2;
        }
        if(m_lfile.size() == 0) {
            sftp_close(wf);
            m_lfile.close();
            return -3;
        }

        if(fileOffset > 0) {
            if(sftp_seek64(wf, fileOffset) != SSH_OK) {
                sftp_close(wf);
                return -4;
            }
            if(!m_lfile.seek(fileOffset)) {
                sftp_close(wf);
                return -5;
            }
        }
        m_rfile = wf;
        m_total = m_lfile.size();
        m_percent =  m_lfile.pos() * 100 / m_total;
        m_speedPerSecond = 0;
        m_tickLast = QDateTime::currentSecsSinceEpoch();
        emit progress(MT_FTP_UPLOAD, m_percent, m_userData);
        return uploadNext(user) ? 1 : 0;
    }

    int _runUploading(QVariantMap& user) {
        if(m_abort) {
            return -999;
        }
        if(m_rfile == nullptr) {
            return -1;
        }
        if(!m_lfile.isOpen()) {
            return -1;
        }
        if(m_lfile.atEnd()) {
           return 0;
        }
        char buffer[MAX_BUFFER];
        int n = m_lfile.read(buffer, MAX_BUFFER);
        int err = sftp_write(m_rfile, buffer, n);
        if(err != n) {
            return -2;
        }
        m_speedPerSecond += n;
        int v =  m_lfile.pos() * 100 / m_total;
        int now = QDateTime::currentSecsSinceEpoch();
        if(now != m_tickLast) {
            m_percent = v;
            m_userData.insert("speed", m_speedPerSecond);
            m_speedPerSecond = 0;
            m_tickLast = now;
            emit progress(MT_FTP_UPLOAD, v, m_userData);
        }else if(v != m_percent) {
            m_percent = v;
            emit progress(MT_FTP_UPLOAD, v, m_userData);
        }
        return uploadNext(user) ? 1 : 0;
    }

    int runFileInfo(const QByteArray& path, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_FILE_INFO, user);
        int code = _runFileInfo(path, m_userData);
        handleCommandFinish(MT_FTP_FILE_INFO, reasonResult(code, m_userData));
        return code;
    }

    int _runFileInfo(const QByteArray& _path, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        sftp_attributes attr = sftp_stat(m_sftp, path);
        if(attr == nullptr) {
            return -1;
        }
        int ownerMax = 0, groupMax = 0, sizeMax = 0, dateMax = 0, nameMax = 0;
        QList<QByteArray> item;
        {
            QByteArray longName(attr->longname);
            QByteArray type = filePermissionToText(attr->type, attr->permissions);
            QByteArray owner(attr->owner);
            QByteArray group(attr->group);
            QByteArray size(QByteArray::number(qint64(attr->size)));
            qint64 mtime = qMax(qint64(attr->mtime), qint64(attr->mtime64));
            if(mtime < 10) {
                mtime = qMax(qint64(attr->atime), qint64(attr->atime64));
            }
            //QDateTime dt = QDateTime::fromMSecsSinceEpoch(mtime * 1000);
            //QByteArray date = dt.toString("dd.MM.yyyy hh:mm:ss").toLatin1();
            QByteArray date = QByteArray::number(mtime);
            QByteArray name(attr->name);
            if(ownerMax < owner.length()) {
                ownerMax = owner.length();
            }
            if(groupMax < group.length()) {
                groupMax = group.length();
            }
            if(sizeMax < size.length()) {
                sizeMax = size.length();
            }
            if(dateMax < date.length()) {
                dateMax = date.length();
            }
            if(nameMax < name.length()) {
                nameMax = name.length();
            }
            item.append(longName);
            item.append(type);
            item.append(owner);
            item.append(group);
            item.append(size);
            item.append(date);
            item.append(name);
        }
        sftp_attributes_free(attr);
        QVariantMap mdata;
        {
            QByteArray longName = item.at(0);
            mdata.insert("longName", item.at(0));
            QString label = item.at(1);
            mdata.insert("type", QString("%1").arg(label.at(0)));
            mdata.insert("permission", label);
            {
                QByteArray owner = item.at(2);
                mdata.insert("owner", owner);
                int nleft = (ownerMax + 7) / 8 * 8 + 1 - owner.length();
                for(int i = 0; i < nleft; i++) {
                    owner.prepend(QChar::Space);
                }
                label.append(owner);
            }
            {
                QByteArray group = item.at(3);
                mdata.insert("group", group);
                int nleft = (groupMax + 7) / 8 * 8 + 1 - group.length();
                for(int i = 0; i < nleft; i++) {
                    group.prepend(QChar::Space);
                }
                label.append(group);
            }
            {
                QByteArray size = item.at(4);
                mdata.insert("size", size);
                int nleft = (sizeMax + 7) / 8 * 8 + 1 - size.length();
                for(int i = 0; i < nleft; i++) {
                    size.prepend(QChar::Space);
                }
                label.append(size);
            }
            {
                QByteArray date = item.at(5);
                mdata.insert("date", date);
                int nleft = (dateMax + 7) / 8 * 8 + 1 - date.length();
                for(int i = 0; i < nleft; i++) {
                    date.prepend(QChar::Space);
                }
                label.append(date);
            }
            {
                QByteArray name = item.at(6);
                mdata.insert("name", name);
                int nleft = (nameMax + 7) / 8 * 8 + 1 - name.length();
                for(int i = 0; i < nleft; i++) {
                    name.prepend(QChar::Space);
                }
                label.append(name);
            }
        }
        char *tmp = sftp_canonicalize_path(m_sftp, path);
        if(tmp == nullptr) {
            return -2;
        }
        mdata.insert("absPath", QByteArray(tmp));
        user.insert("fileInfo", mdata);
        ssh_string_free_char(tmp);
        return 0;
    }

    int runUnlink(const QByteArray& path, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_UNLINK, user);
        int code = _runUnlink(path, m_userData);
        handleCommandFinish(MT_FTP_UNLINK, reasonResult(code, m_userData));
        return code;
    }

    int _runUnlink(const QByteArray& _path, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        if(sftp_unlink(m_sftp, path) != SSH_OK) {
            return -2;
        }
        return 0;
    }

    int runRmDir(const QByteArray& path, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        m_rmDirNext.clear();
        handleCommandStart(MT_FTP_RMDIR, user);
        m_opAsync = eRmDir;
        int code = _runRmDir(path, true, m_userData);
        if(code <= 0) {
            m_opAsync = eNone;
            handleCommandFinish(MT_FTP_RMDIR, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runRmDir(const QByteArray& _path, bool isDir, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        if(!isDir) {
            if(sftp_unlink(m_sftp, path) != SSH_OK) {
                return -2;
            }
        }else{
            sftp_dir dir = sftp_opendir(m_sftp, path);
            if(dir == nullptr) {
                return -3;
            }
            sftp_attributes attr;
            QList<QVariantMap> files;
            while ((attr = sftp_readdir(m_sftp, dir)) != nullptr && !m_abort){
                QByteArray name = attr->name;
                if(name == "." || name == "..") {
                    continue;
                }
                QVariantMap dm;
                dm.insert("filePath", path + "/" + name);
                dm.insert("isDir", attr->type == 2);
                files.append(dm);
                sftp_attributes_free(attr);
            }
            if (sftp_closedir(dir) != SSH_NO_ERROR) {
                return -4;
            }
            if(m_abort) {
                return -999;
            }
            if(files.isEmpty()) {
                if(sftp_rmdir(m_sftp, path) != SSH_OK) {
                    return -5;
                }
            }else{
                QVariantMap dm;
                dm.insert("filePath", path);
                dm.insert("isDir", true);
                m_rmDirNext.append(dm);
                m_rmDirNext.append(files);
            }
        }
        return rmDirNext(user) ? 1 : 0;
    }

    int runMkDir(const QByteArray& path, int mode, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_MKDIR, user);
        int code = _runMkDir(path, mode, m_userData);
        handleCommandFinish(MT_FTP_MKDIR, mkDirResult(code, user));
        return code;
    }

    int _runMkDir(const QByteArray& path, int mode, QVariantMap& user) {
        if(sftp_mkdir(m_sftp, path, mode) != SSH_OK) {
            return -1;
        }
        return 0;
    }    

    int runMkPath(const QByteArray& path, int mode, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_MKPATH, user);
        int code = _runMkPath(path, mode, m_userData);
        handleCommandFinish(MT_FTP_MKPATH, reasonResult(code, m_userData));
        return code;
    }

    int _runMkPath(const QByteArray& _path, int mode, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        QByteArrayList names = path.split('/');
        if(names.isEmpty()) {
            return 0;
        }
        QByteArray mkpath = names.takeFirst();
        for(int i = 0; i < names.length(); i++) {
            QByteArray tmp = names.at(i);
            if(tmp.isEmpty()) {
                continue;
            }
            mkpath = mkpath + "/" + tmp;
            if(sftp_mkdir(m_sftp, mkpath, mode) != SSH_OK) {
                int err = sftp_get_error(m_sftp);
                if(err == SSH_FX_FILE_ALREADY_EXISTS) {
                    continue;
                }
                return 1;
            }
        }

        return 0;
    }

    int runRename(const QByteArray& nameOld, const QByteArray& nameNew, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_RENAME, user);
        int code = _runRename(nameOld, nameNew, m_userData);
        handleCommandFinish(MT_FTP_RENAME, reasonResult(code, m_userData));
        return code;
    }

    int _runRename(const QByteArray& _nameOld, const QByteArray& _nameNew, QVariantMap& user) {
        QByteArray nameOld = canonicalizePath(_nameOld);
        if(nameOld.isEmpty()) {
            return -1;
        }
        QByteArray nameNew = canonicalizePath(_nameNew);
        if(nameNew.isEmpty()) {
            return -1;
        }
        int err = sftp_rename(m_sftp, nameOld, nameNew);
        if(err == SSH_NO_ERROR) {
            return 0;
        }
        return -1;
    }

    int runSymlink(const QByteArray& target, const QByteArray& dest, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_SYMLINK, user);
        int code = _runSymlink(target, dest, m_userData);
        handleCommandFinish(MT_FTP_SYMLINK, reasonResult(code, m_userData));
        return code;
    }

    int _runSymlink(const QByteArray& _target, const QByteArray& _dest, QVariantMap& user) {
        QByteArray target = canonicalizePath(_target);
        if(target.isEmpty()) {
            return -1;
        }
        QByteArray dest = canonicalizePath(_dest);
        if(dest.isEmpty()) {
            return -1;
        }
        int err = sftp_symlink(m_sftp, target, dest);
        if(err == SSH_NO_ERROR) {
            return 0;
        }
        return -1;
    }

    int runChmod(const QByteArray& path, int permission, bool subdirs, const QVariantMap& user) {
        m_abort = false;
        m_userData = user;
        m_errorString.clear();
        handleCommandStart(MT_FTP_CHMOD, user);
        m_chmodFolderNext.clear();
        m_opAsync = eChmodFile;
        int code = _runChmod(path, permission, subdirs, m_userData);
        if(code <= 0) {
            m_opAsync = eNone;
            m_chmodFolderNext.clear();
            handleCommandFinish(MT_FTP_CHMOD, reasonResult(code, m_userData));
        }
        return code;
    }

    int _runChmod(const QByteArray& _path, int permission, bool subdirs, QVariantMap& user) {
        QByteArray path = canonicalizePath(_path);
        if(path.isEmpty()) {
            return -1;
        }
        int err = sftp_chmod(m_sftp, path, permission);
        if(!subdirs) {
            if(err == SSH_NO_ERROR) {
                return 0;
            }
            return -2;
        }
        if(err != SSH_NO_ERROR) {
            return -3;
        }
        sftp_dir dir = sftp_opendir(m_sftp, path);
        if(dir == nullptr) {
            return -4;
        }
        sftp_attributes attr;
        QByteArrayList files;
        while ((attr = sftp_readdir(m_sftp, dir)) != nullptr && !m_abort){
            QByteArray name = attr->name;
            if(name == "." || name == "..") {
                continue;
            }
            if(attr->type == 2) {
                ChmodFolderNext next;
                next.path = path + "/" + name;
                next.permission = permission;
                m_chmodFolderNext.append(next);
            }else{
                QByteArray filePath = path + "/" + name;
                files.append(filePath);
            }
            sftp_attributes_free(attr);
        }
        if (sftp_closedir(dir) != SSH_NO_ERROR) {
            return -3;
        }
        if(m_abort) {
            return -999;
        }
        for(auto it = files.begin(); it != files.end(); it++) {
            QByteArray file = *it;
            int err = sftp_chmod(m_sftp, file, permission);
            if(err == SSH_REQUEST_DENIED || err == SSH_NO_ERROR) {
                continue;
            }
            return -3;
        }

        return chmodFolderNext(user) ? 1 : 0;
    }


    virtual bool handleOpen(void *session){
        sftp_session sftp = sftp_new(ssh_session(session));
        if (sftp == nullptr) {
            handleError(tr("Error allocating SFTP session: %1").arg(ssh_get_error(ssh_session(session))), QVariantMap());
            return false;
        }
        int n = 0;
        do{
            char msg[128];
            n = ssh_channel_read_timeout(sftp->channel, msg, 128, 0, 100);
        }while(n > 0);
        do{
            char msg[128];
            n = ssh_channel_read_timeout(sftp->channel, msg, 128, 1, 100);
        }while(n > 0);
        if(sftp_init(sftp) != SSH_OK) {
            handleError(tr("Error initializing SFTP session: %1").arg(ssh_get_error(sftp)), QVariantMap());
            return false;
        }
        m_sftp = sftp;
        m_channel = m_sftp->channel;
        return true;
    }

    virtual void handleClose(int code){
        int err = sftp_get_error(m_sftp);
        m_lastCommandExitStatus = err == SSH_FX_OK ? 0 : -1;
        if(m_rfile) {
            sftp_close(m_rfile);
            m_rfile = nullptr;
        }
        if(m_sftp) {
            sftp_free(m_sftp);
            m_sftp = nullptr;
        }
        m_lfile.close();
        emit finishArrived(code);
    }

    virtual bool handleRead(){
        if(m_opAsync == eDownload){
            int n = _runDownloading();
            if(n != SSH_AGAIN){
                if(n <= 0) {
                    m_opAsync = eNone;
                    sftp_close(m_rfile);
                    m_rfile = nullptr;
                    m_lfile.close();
                    handleCommandFinish(MT_FTP_DOWNLOAD, reasonResult(n, m_userData));
                }
            }
            return true;
        }
        return true;
    }

    virtual bool handleRequest(struct MsgRequest& msg) {
        if(m_sftp == nullptr) {
            return false;
        }
        try {
            if(!isFtpFatal()) {
                _handleRequest(msg);
            }
            return !isFtpFatal();
        } catch (...) {
            return false;
        }
    }

    virtual bool _handleRequest(struct MsgRequest& msg) {
        int type = msg.type;
        QByteArray data = msg.data;
        if(type == MT_FTP_UPLOADNEXT) {
            QDataStream ds(data);
            QVariantMap user;
            ds >> user;
            int n = _runUploading(user);
            if(n <= 0){
                if(m_rfile) {
                    sftp_close(m_rfile);
                    m_rfile = nullptr;
                }
                m_lfile.close();
                m_opAsync = eNone;
                handleCommandFinish(MT_FTP_UPLOAD, reasonResult(n, user));
            }
            return true;
        }else if(type == MT_FTP_LISTFILENEXT) {
            QDataStream ds(data);
            QVariantMap user;
            QByteArray path;
            ds >> path >> user;
            int n = _runListFile(path, user);
            if(n <= 0) {
                m_listFileNext.clear();
                m_opAsync = eNone;
                handleCommandFinish(MT_FTP_LISTFILE, reasonResult(n, user));
            }
            return true;
        }else if(type == MT_FTP_CHMODNEXT) {
            QDataStream ds(data);
            QVariantMap user;
            QByteArray path;
            int permission;
            ds >> path >> permission >> user;
            int n = _runChmod(path, permission, true, user);
            if(n <= 0) {
                m_chmodFolderNext.clear();
                m_opAsync = eNone;
                handleCommandFinish(MT_FTP_CHMOD, reasonResult(n, user));
            }
            return true;
        }else if(type == MT_FTP_REMOVEFILENEXT) {
            QDataStream ds(data);
            QVariantMap user;
            QByteArray path;
            bool isDir;
            ds >> path >> isDir >> user;
            int n = _runRmDir(path, isDir, user);
            if(n <= 0) {
                m_rmDirNext.clear();
                m_opAsync = eNone;
                handleCommandFinish(MT_FTP_RMDIR, reasonResult(n, user));
            }
            return true;
        }else if(type == MT_FTP_ABORT) {
            m_abort = true;
            return true;
        }

        if(m_opAsync != eNone) {
            m_asyncTasks.append(msg);
            return true;
        }

        if(type == MT_FTP_LISTFILE) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            ds >> path >> user;
            if(path.isEmpty()) {
                path = "~";
            }
            return runListFile(path, user);
        }else if(type == MT_FTP_OPENDIR) {
            QDataStream ds(data);
            QStringList paths;
            QVariantMap user;
            ds >> paths >> user;
            if(paths.isEmpty()) {
                paths.append("~");
            }
            return runOpenDir(paths, user);
        }else if(type == MT_FTP_MKDIR) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            int mode;
            ds >> path >> mode >> user;
            return runMkDir(path, mode, user);
        }else if(type == MT_FTP_MKPATH) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            int mode;
            ds >> path >> mode >> user;
            return runMkPath(path, mode, user);
        }else if(type == MT_FTP_RENAME) {
            QDataStream ds(data);
            QVariantMap user;
            QByteArray nameOld, nameNew;
            ds >> nameOld >> nameNew >> user;
            return runRename(nameOld, nameNew, user);
        }else if(type == MT_FTP_CHMOD) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            int permission;
            bool subdirs;
            ds >> path >> permission >> subdirs >> user;
            return runChmod(path, permission, subdirs, user);
        }else if(type == MT_FTP_RMDIR) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            ds >> path >> user;
            return runRmDir(path, user);
        }else if(type == MT_FTP_UNLINK) {
            QDataStream ds(data);
            QByteArray path;
            QByteArray name;
            QVariantMap user;
            ds >> path >> name;
            return runUnlink(path, user);
        }else if(type == MT_FTP_UPLOAD) {
            QDataStream ds(data);
            QByteArray local;
            QByteArray remote;
            QVariantMap user;
            int policy;
            ds >> local >> remote >> policy >> user;
            return runUpload(local, remote, policy, user);
        }else if(type == MT_FTP_DOWNLOAD) {
            QDataStream ds(data);
            QByteArray local;
            QByteArray remote;
            QVariantMap user;
            int policy;
            QString logFile;
            ds >> remote >> local >> policy >> user;
            return runDownload(remote, local, policy, user);
        }else if(type == MT_FTP_FILE_INFO) {
            QDataStream ds(data);
            QByteArray path;
            QVariantMap user;
            ds >> path >> user;
            return runFileInfo(path, user);
        }else if(type == MT_FTP_READ_FILE_CONTENT) {
            QDataStream ds(data);
            QByteArray path;
            qint64 offset, maxSize;
            QVariantMap user;
            ds >> path >> offset >> maxSize >> user;
            return runFileContent(path, offset, maxSize, user);
        }else if(type == MT_FTP_WRITE_FILE_CONTENT) {
            QDataStream ds(data);
            QByteArray path;
            QByteArray content;
            QVariantMap user;
            ds >> path >> content >> user;
            return runWriteFileContent(path, content, user);
        }
        return true;
    }

    virtual void* channel() {
        return m_channel;
    }
};

class QSshMultClient : public QSshInteractiveClient
{
private:
    QMap<ssh_channel, QPointer<QWoSshChannel>> m_all;
    QList<QPointer<QWoSshChannel>> tmp;
public:
    explicit QSshMultClient(const TargetInfo& ti, QObject *parent)
        : QSshInteractiveClient(ti, parent) {

    }

    virtual ~QSshMultClient() {
    }

    bool open() {
        return true;
    }

    void removeAllChannel() {
        QMutexLocker locker(m_mtx);
        QList<QPointer<QWoSshChannel>> all = m_all.values();
        for(int i = 0; i < all.length(); i++) {
            QWoSshChannel *chn = all.at(i);
            if(chn) {
                chn->handleClose(0);
            }
        }
        m_all.clear();
        for(int i = 0; i < tmp.length(); i++) {
            QWoSshChannel *chn = tmp.at(i);
            if(chn) {
                chn->handleClose(0);
            }
        }
        tmp.clear();
    }

    void append(QWoSshChannel *cli) {
        QMutexLocker locker(m_mtx);
        push(MT_APPEND, QByteArray(), cli);
        tmp.append(cli);
    }

    void remove(QWoSshChannel* cli) {
        push(MT_REMOVE, QByteArray(), cli);
    }

    void customWrite(QWoSshChannel *cli, const QByteArray& data) {
        push(MT_CUSTOMDATA, data, cli);
    }

    void shellWrite(QWoSshChannel *cli, const QByteArray& data) {
        push(MT_PTYDATA, data, cli);
    }

    void shellSize(QWoSshChannel *cli, int cols, int rows) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << cols << rows;
        push(MT_PTYSIZE, buf, cli);
    }
    void shellSignal(QWoSshChannel *cli, const QByteArray& sig) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << sig;
        push(MT_SIGNAL, buf, cli);
    }
    void shellBreak(QWoSshChannel *cli, int length) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << length;
        push(MT_BREAK, buf, cli);
    }

    void sftpOpenDir(QWoSshChannel *cli, const QStringList& paths, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << paths << user;
        push(MT_FTP_OPENDIR, buf, cli);
    }

    void sftpMkDir(QWoSshChannel *cli, const QString& path, int mode, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << mode << user;
        push(MT_FTP_MKDIR, buf, cli);
    }

    void sftpMkPath(QWoSshChannel *cli, const QString& path, int mode, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << mode << user;
        push(MT_FTP_MKPATH, buf, cli);
    }

    void sftpRename(QWoSshChannel *cli, const QString& nameOld, const QString& nameNew, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << nameOld.toUtf8() << nameNew.toUtf8() << user;
        push(MT_FTP_RENAME, buf, cli);
    }

    void sftpSymlink(QWoSshChannel *cli, const QString& target, const QString& dest, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << target.toUtf8() << dest.toUtf8() << user;
        push(MT_FTP_SYMLINK, buf, cli);
    }


    void sftpChmod(QWoSshChannel *cli, const QString& path, int permission, bool subdirs, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << permission << subdirs << user;
        push(MT_FTP_CHMOD, buf, cli);
    }

    void sftpRmDir(QWoSshChannel *cli, const QString& path, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << user;
        push(MT_FTP_RMDIR, buf, cli);
    }

    void sftpUnlink(QWoSshChannel *cli, const QString &path, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << user;
        push(MT_FTP_UNLINK, buf, cli);
    }

    void sftpFileContent(QWoSshChannel *cli, const QString& remote, qint64 offset, qint64 maxSize, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << remote.toUtf8() << offset << maxSize << user;
        push(MT_FTP_READ_FILE_CONTENT, buf, cli);
    }

    void sftpWriteFileContent(QWoSshChannel *cli, const QString& remote, const QByteArray& content, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << remote.toUtf8() << content << user;
        push(MT_FTP_WRITE_FILE_CONTENT, buf, cli);
    }

    void sftpDownload(QWoSshChannel *cli, const QString& remote, const QString& local, int policy, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << remote.toUtf8() << local.toUtf8() << policy << user;
        push(MT_FTP_DOWNLOAD, buf, cli);
    }

    void sftpUpload(QWoSshChannel *cli, const QString& local, const QString& remote, int policy, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << local.toUtf8() << remote.toUtf8() << policy << user;
        push(MT_FTP_UPLOAD, buf, cli);
    }

    void sftpListFile(QWoSshChannel *cli, const QString& path, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << user;
        push(MT_FTP_LISTFILE, buf, cli);
    }

    void sftpFileInfo(QWoSshChannel *cli, const QString &path, const QVariantMap &user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << user;
        push(MT_FTP_FILE_INFO, buf, cli);
    }

    void internalUploadNext(QWoSshChannel *cli, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << user;
        push(MT_FTP_UPLOADNEXT, buf, cli);
    }

    void internalListFileNext(QWoSshChannel *cli, const QByteArray& path, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path << user;
        push(MT_FTP_LISTFILENEXT, buf, cli);
    }

    void internalChmodFolderNext(QWoSshChannel *cli, const QByteArray& path, int permission, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path << permission << user;
        push(MT_FTP_CHMODNEXT, buf, cli);
    }
    void internalRemoveFileNext(QWoSshChannel *cli, const QByteArray& path, bool isDir, const QVariantMap& user) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path << isDir << user;
        push(MT_FTP_REMOVEFILENEXT, buf, cli);
    }

    void internalAsyncTaskNext(QWoSshChannel *cli, uchar type, const QByteArray &data) {
        push(type, data, cli);
    }

    void sftpAbort(QWoSshChannel *cli) {
        push(MT_FTP_ABORT, QByteArray(), cli);
    }

private:
    int channelAdd(QWoSshChannel *chn) {
        QMutexLocker locker(m_mtx);
        m_all.insert(ssh_channel(chn->channel()), chn);
        tmp.removeAll(chn);
        return m_all.count();
    }

    int channelRemove(QWoSshChannel *chn) {
        QMutexLocker locker(m_mtx);
        m_all.remove(ssh_channel(chn->channel()));
        return m_all.count();
    }

    bool channelValid(QWoSshChannel *chn) {
        ssh_channel rchn = ssh_channel(chn->channel());
        return !(ssh_channel_is_eof(rchn) || ssh_channel_is_closed(rchn));
    }

    int channelCount() {
        QMutexLocker locker(m_mtx);
        return m_all.count();
    }

    void run() {
        int ret = running();
        removeAllChannel();
        qDebug() << "ret" << ret;
    }

    int running() {
        fd_set rfds;
        push(MT_WAITUP);        
        while(1) {
            timeval tm={3,0};
            ssh_channel in[MAX_CHANNEL]={0};
            ssh_channel out[MAX_CHANNEL]={0};
            FD_ZERO(&rfds);
            FD_SET(m_msgRead, &rfds);
            int fdmax = m_msgRead;
            int i = 0;
            for(QMap<ssh_channel, QPointer<QWoSshChannel>>::iterator iter = m_all.begin(); iter != m_all.end(); iter++) {                
                in[i++] = iter.key();
            }            
            ssh_select(in, out, fdmax+1, &rfds, &tm);
            bool done = false;
            if(FD_ISSET(m_msgRead, &rfds)) {
                char type;
                done = true;
                if(recv(m_msgRead, (char*)&type, 1, 0) > 0) {
                    struct MsgRequest msg;
                    while(pop(&msg)) {
                        QWoSshChannel *chn = qobject_cast<QWoSshChannel*>(msg.other);
                        if(chn != nullptr) {
                            if(msg.type == MT_APPEND) {
                                if(chn->handleOpen(m_session)){
                                    channelAdd(chn);
                                }
                            }else if(msg.type == MT_REMOVE){
                                channelRemove(chn);
                                chn->handleClose(0);
                            }else if(!chn->handleRequest(msg)) {
                                channelRemove(chn);
                                chn->handleClose(-3);
                            }
                        }else if(msg.type == MT_EXIT) {
                            return 0;
                        }
                    }
                }else{
                    return -1;
                }
            }
            for(int i = 0; i < MAX_CHANNEL && out[i] != 0; i++) {
                done = true;
                QWoSshChannel *chn = m_all.value(out[i]);
                if(chn == nullptr) {
                    m_all.remove(out[i]);
                }else if(!chn->handleRead()) {
                    channelRemove(chn);
                    chn->handleClose(-1);
                }else{
                    if(!channelValid(chn)) {
                        channelRemove(chn);
                        chn->handleClose(-1);
                    }
                }
            }
            if(!done) {
                if(channelCount() == 0) {
                    return 0;
                }
                ssh_send_keepalive(m_session);
            }
        }
        return 0;
    }
};

QWoSSHConnection::QWoSSHConnection(QObject *parent)
    : QThread(parent)
    , m_conn(nullptr)
    , m_listenSocket(0)
    , m_connectionState(-1)
{
    QObject::connect(this, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
}

QWoSSHConnection::~QWoSSHConnection()
{
    wait();
}

QString QWoSSHConnection::hostName() const
{
    if(m_conn == nullptr) {
        return "";
    }
    return m_conn->name();
}


void QWoSSHConnection::append(QWoSshChannel *cli)
{
    if(m_conn) {
        m_conn->append(cli);
    }
}

void QWoSSHConnection::remove(QWoSshChannel *cli)
{
    if(m_conn == nullptr) {
        return;
    }
    if(m_connectionState < 1) {
        m_conn->stop();
    }else{
        m_conn->remove(cli);
    }
}


bool QWoSSHConnection::start(const QString &host)
{
    if(hasRunning()) {
        return false;
    }
    if(!init(host)) {
        return false;
    }
    QThread::start();
    emit connectionStart();
    return true;
}

bool QWoSSHConnection::start(const HostInfo &hi)
{
    if(hasRunning()) {
        return false;
    }
    if(!init(hi)) {
        return false;
    }
    QThread::start();
    emit connectionStart();
    return true;
}

void QWoSSHConnection::stop()
{
    if(m_listenSocket > 0) {
        myclosesocket(socket_t(m_listenSocket));
        m_listenSocket = 0;
    }

    if(m_conn) {
        if(m_conn->isRunning()) {
            m_conn->stop();
        }else if(!isRunning()){
            m_conn->removeAllChannel();
        }
    }
    for(int i = 0; i < m_proxys.length(); i++) {
        QSshClient *cli = m_proxys[i];
        cli->stop();
    }
}

void QWoSSHConnection::setInputResult(const QString &pass)
{
    if(m_input) {
        m_input->setInputResult(pass);
        m_input = nullptr;
    }
}

void QWoSSHConnection::customWrite(QWoSshChannel *cli, const QByteArray &buf)
{
    if(m_conn) {
        m_conn->customWrite(cli, buf);
    }
}

void QWoSSHConnection::shellWrite(QWoSshChannel*cli, const QByteArray &buf)
{
    if(m_conn) {
        m_conn->shellWrite(cli, buf);
    }
}

void QWoSSHConnection::shellSize(QWoSshChannel*cli, int cols, int rows)
{
    if(m_conn) {
        m_conn->shellSize(cli, cols, rows);
    }
}

void QWoSSHConnection::shellSignal(QWoSshChannel *cli, const QByteArray &sig)
{
    if(m_conn) {
        m_conn->shellSignal(cli, sig);
    }
}

void QWoSSHConnection::shellBreak(QWoSshChannel *cli, int length)
{
    if(m_conn) {
        m_conn->shellBreak(cli, length);
    }
}

void QWoSSHConnection::sftpOpenDir(QWoSshChannel *cli, const QStringList &paths, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpOpenDir(cli, paths, user);
    }
}

void QWoSSHConnection::sftpMkDir(QWoSshChannel *cli, const QString &path, int mode, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpMkDir(cli, path, mode, user);
    }
}

void QWoSSHConnection::sftpMkPath(QWoSshChannel *cli, const QString &path, int mode, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpMkPath(cli, path, mode, user);
    }
}

void QWoSSHConnection::sftpRename(QWoSshChannel *cli, const QString& nameOld, const QString& nameNew, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpRename(cli, nameOld, nameNew, user);
    }
}

void QWoSSHConnection::sftpSymlink(QWoSshChannel *cli, const QString &target, const QString &dest, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpSymlink(cli, target, dest, user);
    }
}

void QWoSSHConnection::sftpChmod(QWoSshChannel *cli, const QString &path, int permission, bool subdirs, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpChmod(cli, path, permission, subdirs, user);
    }
}

void QWoSSHConnection::sftpRmDir(QWoSshChannel *cli, const QString &path, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpRmDir(cli, path, user);
    }
}

void QWoSSHConnection::sftpUnlink(QWoSshChannel *cli, const QString &path, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpUnlink(cli, path, user);
    }
}

void QWoSSHConnection::sftpFileContent(QWoSshChannel *cli, const QString &remote, qint64 offset, qint64 maxSize, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpFileContent(cli, remote, offset, maxSize, user);
    }
}

void QWoSSHConnection::sftpWriteFileContent(QWoSshChannel *cli, const QString &remote, const QByteArray &content, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpWriteFileContent(cli, remote, content, user);
    }
}

void QWoSSHConnection::sftpDownload(QWoSshChannel *cli, const QString &remote, const QString &local, int policy, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpDownload(cli, remote, local, policy, user);
    }
}

void QWoSSHConnection::sftpUpload(QWoSshChannel *cli, const QString &local, const QString &remote, int policy, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpUpload(cli, local, remote, policy, user);
    }
}

void QWoSSHConnection::sftpListFile(QWoSshChannel *cli, const QString &path, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpListFile(cli, path, user);
    }
}

void QWoSSHConnection::sftpFileInfo(QWoSshChannel *cli, const QString &path, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpFileInfo(cli, path, user);
    }
}

void QWoSSHConnection::sftpAbort(QWoSshChannel *cli)
{
    if(m_conn) {
        m_conn->sftpAbort(cli);
    }
}

void QWoSSHConnection::internalUploadNext(QWoSshChannel *cli, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->internalUploadNext(cli, user);
    }
}

void QWoSSHConnection::internalListFileNext(QWoSshChannel *cli, const QByteArray& path, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->internalListFileNext(cli, path, user);
    }
}

void QWoSSHConnection::internalChmodFolderNext(QWoSshChannel *cli, const QByteArray &path, int permission, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->internalChmodFolderNext(cli, path, permission, user);
    }
}

void QWoSSHConnection::internalRemoveFileNext(QWoSshChannel *cli, const QByteArray &path, bool isDir, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->internalRemoveFileNext(cli, path, isDir, user);
    }
}

void QWoSSHConnection::internalAsyncTaskNext(QWoSshChannel *cli, uchar type, const QByteArray &data)
{
    if(m_conn) {
        m_conn->internalAsyncTaskNext(cli, type, data);
    }
}

void QWoSSHConnection::onInputArrived(const QString &host, const QString &prompt, bool show)
{
    QSshClient *cli = qobject_cast<QSshClient*>(sender());
    m_input = cli;
    emit inputArrived(host, prompt, show);
}

void QWoSSHConnection::onFinished()
{
    QSshClient *cli = qobject_cast<QSshClient*>(sender());
    if(cli) {
        int code = cli->exitCode();
        emit finishArrived(code);
    }
}

void QWoSSHConnection::onConnectionFinished(bool ok)
{
    m_connectionState = ok ? 1 : 0;
    if(!ok) {
        emit finishArrived(-1);
    }
}

bool QWoSSHConnection::hasRunning()
{
    if(isRunning()) {
        return true;
    }
    if(m_conn && m_conn->isRunning()) {
        return true;
    }
    for(int i = 0; i < m_proxys.length(); i++) {
        QSshClient *cli = m_proxys.at(i);
        if(cli->isRunning()) {
            return true;
        }
    }
    return false;
}

static QSshClient::TargetInfo qsshToTarget(const HostInfo &hi, bool forw)
{
    QSshClient::TargetInfo ti;
    ti.forward = forw;
    ti.name = hi.name;
    ti.host = hi.host;
    ti.port = ushort(hi.port);
    ti.user = hi.user;
    ti.password = hi.password;
    ti.identityFileContent = hi.identityContent.toUtf8();
    return ti;
}

bool QWoSSHConnection::init(const QString &host)
{
    // it's danger to remove the follow line for mult thread.
    static bool initSshAgent = QWoLocalSshAgentServer::instance();
    QList<HostInfo> his = QWoSshConf::instance()->proxyJumpers(host);
    if(his.isEmpty()) {
        return false;
    }
    if(m_listenSocket > 0) {
        myclosesocket(socket_t(m_listenSocket));
    }
    m_listenSocket = 0;
    m_listenPort = 0;
    socket_t server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    ushort port = QWoUtils::listenLocal(server, 20327);
    if(port == 0) {
        myclosesocket(server);
        return false;
    }
    if(his.length() * 2 >= 10) {
        myclosesocket(server);
        return false;
    }
    m_listenSocket = int(server);
    m_listenPort = port;
    const HostInfo &hi = his.takeFirst();
    QSshClient::TargetInfo ti = qsshToTarget(hi, !his.isEmpty());
    m_conn = new QSshMultClient(ti, this);
    if(!m_conn->init(m_listenSocket, m_listenPort)) {
        return false;
    }
    for(int i = 0; i < his.length(); i++) {
        const HostInfo& hi = his[i];
        QSshClient::TargetInfo ti = qsshToTarget(hi, i < his.length() - 1);
        QSshProxyClient *cli = new QPowerProxyClient(ti, this);
        if(!cli->init(m_listenSocket, m_listenPort)) {
            return false;
        }
        QObject::connect(cli, SIGNAL(finished()), this, SLOT(onFinished()));
        QObject::connect(cli, SIGNAL(errorArrived(QString,QVariantMap)), this, SIGNAL(errorArrived(QString,QVariantMap)));
        QObject::connect(cli, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
        QObject::connect(cli, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
        m_proxys.append(cli);
    }
    QObject::connect(m_conn, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(m_conn, SIGNAL(errorArrived(QString,QVariantMap)), this, SIGNAL(errorArrived(QString,QVariantMap)));
    QObject::connect(m_conn, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
    QObject::connect(m_conn, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    return true;
}

bool QWoSSHConnection::init(const HostInfo &hi)
{
    if(m_listenSocket > 0) {
        myclosesocket(socket_t(m_listenSocket));
    }
    m_listenSocket = 0;
    m_listenPort = 0;
    socket_t server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    ushort port = QWoUtils::listenLocal(server, 20327);
    if(port == 0) {
        myclosesocket(server);
        return false;
    }
    m_listenSocket = int(server);
    m_listenPort = port;
    QSshClient::TargetInfo ti = qsshToTarget(hi, false);
    m_conn = new QSshMultClient(ti, this);
    if(!m_conn->init(m_listenSocket, m_listenPort)) {
        return false;
    }
    QObject::connect(m_conn, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(m_conn, SIGNAL(errorArrived(QString,QVariantMap)), this, SIGNAL(errorArrived(QString,QVariantMap)));
    QObject::connect(m_conn, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
    QObject::connect(m_conn, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    return true;
}

void QWoSSHConnection::run()
{
    bool ok = running();
    if(!ok) {
        m_conn->removeAllChannel();
    }
    emit connectionFinished(ok);
}

bool QWoSSHConnection::running()
{
    if(m_proxys.length() > 0) {
        if(!connectToProxy(0, m_conn->host(), m_conn->port())){
            return false;
        }
    }
    if(!m_conn->setupConnection()) {
        return false;
    }

    if(!m_conn->open()) {
        return false;
    }

    m_conn->start();
    return true;
}

bool QWoSSHConnection::connectToProxy(int i, const QString &host, ushort port)
{
    if(i + 1 < m_proxys.length()) {
        QSshProxyClient *cli = m_proxys.at(i);
        if(!connectToProxy(i+1, cli->host(), cli->port())) {
            return false;
        }
    }
    QSshProxyClient *cli = m_proxys[i];
    if(!cli->setupConnection()) {
        return false;
    }
    if(!cli->openFoward(host, port)) {
        return false;
    }
    cli->start();
    return true;
}

QWoSshChannel::QWoSshChannel(QObject *parent)
    : QObject(parent)
    , m_lastCommandExitStatus(-1)
{
}

QWoSshChannel::~QWoSshChannel()
{
}

bool QWoSshChannel::start(const QString &host, int gid)
{
    m_host = host;
    QWoSshFactory *factory = QWoSshFactory::instance();
    bool ct = false;
    m_conn = factory->get(gid, &ct);
    if(ct) {
        QObject::connect(m_conn, SIGNAL(errorArrived(QString,QVariantMap)), this, SIGNAL(errorArrived(QString,QVariantMap)));
        QObject::connect(m_conn, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
        QObject::connect(m_conn, SIGNAL(inputArrived(QString,QString,bool)), this, SIGNAL(inputArrived(QString,QString,bool)));
        QObject::connect(m_conn, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
        QObject::connect(m_conn, SIGNAL(connectionFinished(bool)), this, SIGNAL(connectionFinished(bool)));
        if(!m_conn->start(host)) {
            return false;
        }
    }else{
        QObject::connect(m_conn, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
        QObject::connect(m_conn, SIGNAL(connectionFinished(bool)), this, SIGNAL(connectionFinished(bool)));
    }
    init();
    m_conn->append(this);
    return true;
}

bool QWoSshChannel::start(const HostInfo &hi, int gid)
{
    m_host = hi.name;
    QWoSshFactory *factory = QWoSshFactory::instance();
    bool ct = false;
    m_conn = factory->get(gid, &ct);
    if(ct) {
        QObject::connect(m_conn, SIGNAL(errorArrived(QString,QVariantMap)), this, SIGNAL(errorArrived(QString,QVariantMap)));
        QObject::connect(m_conn, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
        QObject::connect(m_conn, SIGNAL(inputArrived(QString,QString,bool)), this, SIGNAL(inputArrived(QString,QString,bool)));
        QObject::connect(m_conn, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
        QObject::connect(m_conn, SIGNAL(connectionFinished(bool)), this, SIGNAL(connectionFinished(bool)));
        if(!m_conn->start(hi)) {
            return false;
        }
    }else{
        QObject::connect(m_conn, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
        QObject::connect(m_conn, SIGNAL(connectionFinished(bool)), this, SIGNAL(connectionFinished(bool)));
    }
    init();
    m_conn->append(this);
    return true;
}

void QWoSshChannel::stop()
{
    if(m_conn){
        m_conn->remove(this);
    }else{
        emit finishArrived(-1);
    }
}

void QWoSshChannel::setInputResult(const QString &pass)
{
    if(m_conn) {
        m_conn->setInputResult(pass);
    }
}

bool QWoSshChannel::hasRunning()
{
    if(m_conn) {
        return m_conn->hasRunning();
    }
    return false;
}

int QWoSshChannel::lastCommandExitCode()
{
    return m_lastCommandExitStatus;
}

QWoSSHConnection *QWoSshChannel::connection()
{
    return m_conn;
}

QWoSshShell::QWoSshShell(QObject *parent)
    : QWoSshChannel(parent)
{
    m_cacheCount = 0;
    QObject::connect(this, SIGNAL(internalDataArrived(QByteArray)), this, SLOT(onInternalDataArrived(QByteArray)));
}

QWoSshShell::~QWoSshShell()
{

}


void QWoSshShell::write(const QByteArray &buf)
{
    if(m_conn) {
        m_conn->shellWrite(this, buf);
    }
}

void QWoSshShell::updateSize(int cols, int rows)
{
    if(m_conn) {
        m_conn->shellSize(this, cols, rows);
    }
}

void QWoSshShell::sendBreak(int length)
{
    if(m_conn) {
        m_conn->shellBreak(this, length);
    }
}

void QWoSshShell::sendSignal(const QByteArray &sig)
{
    if(m_conn) {
        m_conn->shellSignal(this, sig);
    }
}

#define MAX_CACHE   (30)
void QWoSshShell::emitSendData(const QByteArray &buf)
{
    m_cacheCount++;
    if(m_cacheCount > MAX_CACHE) {
        int cnt = m_cacheCount / MAX_CACHE;
        if(cnt < 10){
            QThread::msleep(cnt * 5);
        }else if(cnt < 30) {
            QThread::msleep(cnt * 10);
        }else if(cnt < 60) {
            QThread::msleep(cnt * 20);
        }else{
            QThread::msleep(cnt * 30);
        }
    }
#if 0
    qDebug() << "emitCount:" << m_cacheCount;
#endif
    emit internalDataArrived(buf);
}

void QWoSshShell::onInternalDataArrived(const QByteArray &buf)
{
    m_cacheCount--;
    emit dataArrived(buf);
}

void QWoSshShell::init()
{

}

QWoSshFtp::QWoSshFtp(QObject *parent)
    : QWoSshChannel(parent)
{
}

QWoSshFtp::~QWoSshFtp()
{

}

void QWoSshFtp::setLogFile(const QString &logFile)
{

}

void QWoSshFtp::openDir(const QStringList &paths, const QVariantMap &user)
{
    if(paths.isEmpty()) {
        return;
    }
    if(m_conn) {
        m_conn->sftpOpenDir(this, paths, user);
    }
}

void QWoSshFtp::openDir(const QString &path, const QVariantMap& user)
{
    QStringList paths;
    paths.append(path);
    openDir(paths, user);
}

void QWoSshFtp::mkDir(const QString &path, int mode, const QVariantMap& user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpMkDir(this, tmp, mode, user);
    }
}

void QWoSshFtp::mkPath(const QString &path, int mode, const QVariantMap &user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpMkPath(this, tmp, mode, user);
    }
}

void QWoSshFtp::rename(QString &nameOld, const QString &nameNew, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpRename(this, nameOld, nameNew, user);
    }
}

void QWoSshFtp::symlink(QString &target, const QString &dest, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpSymlink(this, target, dest, user);
    }
}

void QWoSshFtp::chmod(const QString &path, int permission, bool subdirs, const QVariantMap &user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpChmod(this, tmp, permission, subdirs, user);
    }
}

void QWoSshFtp::rmDir(const QString &path, const QVariantMap& user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpRmDir(this, tmp, user);
    }
}

void QWoSshFtp::unlink(const QString &path, const QVariantMap& user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpUnlink(this, tmp, user);
    }
}

void QWoSshFtp::fileInfo(const QString &path, const QVariantMap &user)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpFileInfo(this, tmp, user);
    }
}

void QWoSshFtp::fileContent(const QString &remote, qint64 offset, qint64 maxSize, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpFileContent(this, remote, offset, maxSize, user);
    }
}

void QWoSshFtp::writeFileContent(const QString &remote, const QByteArray &content, const QVariantMap &user)
{
    if(m_conn) {
        m_conn->sftpWriteFileContent(this, remote, content, user);
    }
}

void QWoSshFtp::download(const QString &remote, const QString &local, TransferPolicy policy, const QVariantMap& user)
{
    QString path = local;
    if(local.startsWith("file://")) {
        QUrl url(local);
        path = url.toLocalFile();
    }
    if(m_conn) {
        m_conn->sftpDownload(this, remote, path, policy, user);
    }
}

void QWoSshFtp::upload(const QString &local, const QString &remote, TransferPolicy policy, const QVariantMap& user)
{
    QString path = local;
    if(local.startsWith("file://")) {
        QUrl url(local);
        path = url.toLocalFile();
    }
    if(m_conn) {
        m_conn->sftpUpload(this, path, remote, policy, user);
    }
}

void QWoSshFtp::listFile(const QString &path, const QVariantMap& user)
{
    if(m_conn) {
        m_conn->sftpListFile(this, path, user);
    }
}

void QWoSshFtp::abort()
{
    if(m_conn) {
        m_conn->sftpAbort(this);
    }
}

//suid, sgid, sticky_bit | user_read, user_write, user_execute | group_read, group_write, group_execute | others_read, others_write, others_execute
#define FILE_PERMISSION_SUID            (1 << 11)
#define FILE_PERMISSION_SGID            (1 << 10)
#define FILE_PERMISSION_STICKY_BIT      (1 << 9)
#define FILE_PERMISSION_USER_READ       (1 << 8)
#define FILE_PERMISSION_USER_WRITE      (1 << 7)
#define FILE_PERMISSION_USER_EXECUTE    (1 << 6)
#define FILE_PERMISSION_GROUP_READ      (1 << 5)
#define FILE_PERMISSION_GROUP_WRITE     (1 << 4)
#define FILE_PERMISSION_GROUP_EXECUTE   (1 << 3)
#define FILE_PERMISSION_OTHER_READ      (1 << 2)
#define FILE_PERMISSION_OTHER_WRITE     (1 << 1)
#define FILE_PERMISSION_OTHER_EXECUTE   (1)
QByteArray QWoSshFtp::filePermissionToText(int type, int flag)
{
    QByteArray buf;
    switch (type) {
    case SSH_FILEXFER_TYPE_REGULAR:
        buf.append('-');
        break;
    case SSH_FILEXFER_TYPE_DIRECTORY:
        buf.append('d');
        break;
    case SSH_FILEXFER_TYPE_SYMLINK:
        buf.append('l');
        break;
    default:
        //buf.append('0'+type);
        buf.append('-');
        break;
    }
    buf.append(flag&FILE_PERMISSION_USER_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_USER_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_USER_EXECUTE ? 'x':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_EXECUTE ? 'x':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_EXECUTE ? 'x':'-');
    return buf;
}


void QWoSshFtp::init()
{
}


QWoSshFactory::QWoSshFactory(QObject *parent)
    : QObject(parent)
{
    QObject::connect(QCoreApplication::instance(), SIGNAL(lastWindowClosed()), this, SLOT(onAboutToQuit()));
}

QWoSshFactory::~QWoSshFactory()
{

}

QWoSshFactory *QWoSshFactory::instance()
{
    static QPointer<QWoSshFactory> factory = new QWoSshFactory();
    return factory;
}

QWoSshShell *QWoSshFactory::createShell(bool cmd)
{
    return new QWoPowerShell(cmd, this);
}

QWoSshFtp *QWoSshFactory::createSftp()
{
    return new QWoPowerSftp(this);
}

void QWoSshFactory::release(QWoSshChannel *obj)
{
    if(obj == nullptr || m_dels.contains(obj)) {
        return;
    }
    //QObject *parent = obj->parent();
    if(obj->parent() != this) {
        obj->setParent(this);
    }
    //QObject *parent2 = obj->parent();
    //qDebug() << "SshFactory::release" << qint64(parent) << qint64(parent2) << qint64(this);
    obj->disconnect();
    QObject::connect(obj, SIGNAL(finishArrived(int)), this, SLOT(onChannelFinishArrived(int)));
    if(!obj->hasRunning()) {
        obj->deleteLater();
        return;
    }
    obj->stop();
    m_dels.append(obj);
}

QWoSSHConnection *QWoSshFactory::get(int gid, bool *pcreated)
{
    QWoSSHConnection *conn = m_objs.value(gid);
    *pcreated = false;
    if(conn == nullptr) {
        *pcreated = true;
        conn = new QWoSSHConnection(this);
        QObject::connect(conn, SIGNAL(finishArrived(int)), this, SLOT(onConnectionFinishArrived(int)));
        m_objs.insert(gid, conn);
    }
    return conn;
}

int QWoSshFactory::groudId(const QString &name)
{
    for(auto it = m_objs.begin(); it != m_objs.end(); it++) {
        int id = it.key();
        QWoSSHConnection *conn = it.value();
        if(conn == nullptr) {
            continue;
        }
        QString nameHit = conn->hostName();
        if(nameHit == name) {
            return id;
        }
    }
    return -1;
}

void QWoSshFactory::onChannelFinishArrived(int)
{
    QWoSshChannel *chn = qobject_cast<QWoSshChannel*>(sender());
    for(int i = 0; i < m_dels.length(); i++) {
        if(m_dels.at(i) == chn) {
            m_dels.removeAt(i);
            chn->deleteLater();
            return;
        }
    }
}

void QWoSshFactory::onConnectionFinishArrived(int)
{
    QWoSSHConnection *conn = qobject_cast<QWoSSHConnection*>(sender());
    for(QMap<int, QPointer<QWoSSHConnection>>::iterator iter = m_objs.begin(); iter != m_objs.end(); iter++) {
        if(iter.value() == conn) {
            conn->stop();
            m_objs.erase(iter);
            conn->deleteLater();
            return;
        }
    }
}

void QWoSshFactory::onAboutToQuit()
{
//    for(QList<QPointer<QWoSSHConnection>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
//        QWoSSHConnection *obj = *iter;
//        if(obj == nullptr) {
//            iter = m_dels.erase(iter);
//            continue;
//        }
//        obj->killAll();
//    }
}

void QWoSshFactory::cleanup()
{
//    for(QList<QPointer<QWoSSHConnection>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
//        QWoSSHConnection *obj = *iter;
//        if(obj == nullptr) {
//            iter = m_dels.erase(iter);
//            continue;
//        }
//        if(!obj->hasRunning()) {
//            obj->deleteLater();
//            iter = m_dels.erase(iter);
//            continue;
//        }
//        obj->stop();
//        iter++;
//    }
}
