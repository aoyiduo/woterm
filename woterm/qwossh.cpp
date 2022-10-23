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

#include "qwoutils.h"
#include "qwosetting.h"

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

#define MT_FTP_OPENDIR      (10)
#define MT_FTP_MKDIR        (11)
#define MT_FTP_RMDIR        (12)
#define MT_FTP_UNLINK       (13)
#define MT_FTP_DOWNLOAD     (14)
#define MT_FTP_UPLOAD       (15)
#define MT_FTP_UPLOADNEXT   (16)
#define MT_FTP_ABORT        (17)


class QSshInteractiveClient : public QSshAuthClient
{
public:
    explicit QSshInteractiveClient(const TargetInfo& ti, QObject *parent)
        : QSshAuthClient(ti, parent){

    }

    virtual bool open() = 0;
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
        if(m_channel) {
            ssh_channel_close(m_channel);
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

    enum ETransfer { eIdle, eUpload, eDownload};
    ETransfer m_transfer;

    QFile m_lfile;
    sftp_file m_rfile;
    qint64 m_cnt, m_total;
    int m_asyncRequest, m_percent;
    QByteArray m_pathCurrent;
    bool m_abort;
public:
    explicit QWoPowerSftp(QObject *parent = nullptr)
     : QWoSshFtp(parent)
     , m_channel(nullptr)
     , m_sftp(nullptr)
     , m_transfer(eIdle)
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

    void uploadNext() {
        if(m_conn) {
            m_conn->internalUploadNext(this);
        }
    }

private:
    void handleError(const QString& err) {
        emit errorArrived(err.toUtf8());
    }

    bool isFtpFatal() {
        int code = sftp_get_error(m_sftp);
        if(code == SSH_FX_NO_CONNECTION || code == SSH_FX_CONNECTION_LOST) {
            return true;
        }
        return false;
    }

    void handleFtpErrorMessage() {
        int code = sftp_get_error(m_sftp);
        switch (code) {
        case SSH_FX_FILE_ALREADY_EXISTS:
            handleError(tr("An attempt to create an already existing file or directory has been made"));
            break;
        case SSH_FX_NO_SUCH_FILE:
        case SSH_FX_NO_SUCH_PATH:
            handleError(tr("No such file or directory path exists"));
            break;
        case SSH_FX_PERMISSION_DENIED:
            handleError(tr("Permission denied"));
            break;
        case SSH_FX_FAILURE:
            handleError(tr("Generic failure"));
            break;
        case SSH_FX_BAD_MESSAGE:
            handleError(tr("Garbage received from server"));
            break;
        case SSH_FX_NO_CONNECTION:
            handleError(tr("No connection has been set up"));
            break;
        case SSH_FX_CONNECTION_LOST:
            handleError(tr("There was a connection, but we lost it"));
            break;
        case SSH_FX_OP_UNSUPPORTED:
            handleError(tr("Operation not supported by the server"));
            break;
        case SSH_FX_INVALID_HANDLE:
            handleError(tr("Invalid file handle"));
            break;
        case SSH_FX_WRITE_PROTECT:
            handleError(tr("We are trying to write on a write-protected filesystem"));
            break;
        case SSH_FX_NO_MEDIA:
            handleError(tr("No media in remote drive"));
            break;
        }
    }

    int runOpenDir(const QByteArray &path) {
        emit commandStart(MT_FTP_OPENDIR);
        int code = _runOpenDir(path);
        emit commandFinish(MT_FTP_OPENDIR);
        return code;
    }

    int _runOpenDir(const QByteArray& path) {
        sftp_dir dir = sftp_opendir(m_sftp, path);
        if(dir == nullptr) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 1;
        }
        sftp_attributes attr;
        QList<QList<QByteArray>> all;
        int ownerMax = 0, groupMax = 0, sizeMax = 0, dateMax = 0, nameMax = 0;
        while ((attr = sftp_readdir(m_sftp, dir)) != nullptr){
            QByteArray longName(attr->longname);
            QByteArray type = QWoUtils::filePermissionToText(attr->type, attr->permissions);
            QByteArray owner(attr->owner);
            QByteArray group(attr->group);
            QByteArray size(QByteArray::number(qint64(attr->size)));
            qint64 mtime = qMax(qint64(attr->mtime), qint64(attr->mtime64));
            if(mtime < 10) {
                mtime = qMax(qint64(attr->atime), qint64(attr->atime64));
            }
            QDateTime dt = QDateTime::fromMSecsSinceEpoch(mtime * 1000);
            QByteArray date = dt.toString("dd.MM.yyyy hh:mm:ss").toLatin1();
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
        if (!sftp_dir_eof(dir)) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
        }
        if (sftp_closedir(dir) != SSH_NO_ERROR) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
        }
        char *tmp = sftp_canonicalize_path(m_sftp, path);
        QVariantMap md;
        md.insert("list", vall);
        md.insert("path", QByteArray(tmp));
        m_pathCurrent = tmp;
        emit response(FTP_TYPE_OPENDIR, md);
        return 0;
    }

    int runDownload(const QByteArray& remote, const QByteArray& local) {
        emit progress(MT_FTP_DOWNLOAD, 0);
        emit commandStart(MT_FTP_DOWNLOAD);
        int code = _runDownload(remote, local);        
        return code;
    }

    int _runDownload(const QByteArray& remote, const QByteArray& local) {
        sftp_attributes attr = sftp_stat(m_sftp, remote);
        if(attr == nullptr) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 0;
        }
        int total = attr->size;
        sftp_attributes_free(attr);
        sftp_file rf = sftp_open(m_sftp, remote, O_RDONLY, 0);
        if(rf == nullptr) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 0;
        }
        m_lfile.setFileName(local);
        if(m_lfile.exists()) {
            sftp_close(rf);
            handleError(tr("Skip it for the local file had exist."));
            return 0;
        }
        if(!m_lfile.open(QFile::WriteOnly)) {
            sftp_close(rf);
            handleError(tr("Failed to open the local file."));
            return 0;
        }
        m_rfile = rf;
        m_transfer = eDownload;
        m_cnt = 0;
        m_percent = 0;
        m_total = total+1;
        sftp_file_set_nonblocking(rf);
        m_asyncRequest = sftp_async_read_begin(rf, MAX_BUFFER);
        return 0;
    }

    int _runDownloading() {
        if(m_abort) {
            return 0;
        }
        QByteArray buf(MAX_BUFFER, Qt::Uninitialized);
        int n = sftp_async_read(m_rfile, buf.data(), buf.length(), m_asyncRequest);
        if(n <= 0){
            return n;
        }
        m_asyncRequest = sftp_async_read_begin(m_rfile, MAX_BUFFER);
        m_cnt += n;
        int v = m_cnt * 100 / m_total;
        if(v != m_percent) {
            m_percent = v;
            emit progress(MT_FTP_DOWNLOAD, v);
        }
        m_lfile.write(buf, n);
        return n;
    }


    int runUpload(const QByteArray& local, const QByteArray& remote) {
        emit progress(MT_FTP_UPLOAD, 0);
        emit commandStart(MT_FTP_UPLOAD);
        int code = _runUpload(local, remote);
        return code;
    }

    int _runUpload(const QByteArray& local, const QByteArray& remote) {
        int access_type = O_WRONLY | O_CREAT | O_TRUNC;
        sftp_file wf = sftp_open(m_sftp, remote, access_type, 0600);
        if(wf == nullptr) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 0;
        }
        m_lfile.setFileName(local);
        if(!m_lfile.open(QFile::ReadOnly)) {
            sftp_close(wf);
            handleError(tr("Failed to open local file."));
            return 0;
        }
        m_rfile = wf;
        m_transfer = eUpload;
        m_cnt = 0;
        m_percent = 0;
        m_total = m_lfile.size()+1;
        uploadNext();
        return 0;
    }

    int _runUploading() {
        if(m_abort) {
            return 0;
        }
        if(!m_lfile.isOpen()) {
            return -1;
        }
        if(m_lfile.atEnd()) {
           return 0;
        }
        char buffer[MAX_BUFFER];
        int n = m_lfile.read(buffer, MAX_BUFFER);
        if(sftp_write(m_rfile, buffer, n) != n) {
            return -1;
        }
        int v =  m_lfile.pos() * 100 / m_total;
        if(v != m_percent) {
            m_percent = v;
            emit progress(MT_FTP_UPLOAD, v);
        }
        uploadNext();
        return n;
    }

    int runUnlink(const QByteArray& path, const QByteArray& name) {
        emit commandStart(MT_FTP_UNLINK);
        int code = _runUnlink(path + "/" + name);
        if(code == 0) {
            code = _runOpenDir(path);
        }
        emit commandFinish(MT_FTP_UNLINK);
        return code;
    }

    int _runUnlink(const QByteArray& path) {
        if(sftp_unlink(m_sftp, path) != SSH_OK) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 1;
        }
        return 0;
    }

    int runRmDir(const QByteArray& path, const QByteArray& name) {
        emit commandStart(MT_FTP_RMDIR);
        int code = _runRmDir(path + "/" + name);
        if(code == 0) {
            code = _runOpenDir(path);
        }
        emit commandFinish(MT_FTP_RMDIR);
        return code;
    }

    int _runRmDir(const QByteArray& path) {
        if(sftp_rmdir(m_sftp, path) != SSH_OK) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 1;
        }
        return 0;
    }

    int runMkDir(const QByteArray& path, const QByteArray& name, int mode) {
        emit commandStart(MT_FTP_MKDIR);
        int code = _runMkDir(path + "/" + name, mode);
        if(code == 0) {
            code = _runOpenDir(path);
        }
        emit commandFinish(MT_FTP_MKDIR);
        return code;
    }

    int _runMkDir(const QByteArray& path, int mode) {
        if(sftp_mkdir(m_sftp, path, mode) != SSH_OK) {
            handleFtpErrorMessage();
            if(isFtpFatal()) {
                return -1;
            }
            return 1;
        }
        return 0;
    }    

    virtual bool handleOpen(void *session){
        sftp_session sftp = sftp_new(ssh_session(session));
        if (sftp == nullptr) {
            handleError(tr("Error allocating SFTP session: %1").arg(ssh_get_error(ssh_session(session))));
            return false;
        }
        if(sftp_init(sftp) != SSH_OK) {
            handleError(tr("Error initializing SFTP session: %1").arg(ssh_get_error(sftp)));
            return false;
        }
        m_sftp = sftp;
        m_channel = m_sftp->channel;
        return true;
    }

    virtual void handleClose(int code){
        if(m_rfile) {
            sftp_close(m_rfile);
            m_rfile = nullptr;
        }
        if(m_sftp) {
            sftp_free(m_sftp);
            m_sftp = nullptr;
        }
        emit finishArrived(code);
    }

    virtual bool handleRead(){
        if(m_transfer == eDownload){
            int n = _runDownloading();
            if( n <= 0) {
                m_transfer = eIdle;
                sftp_close(m_rfile);
                m_rfile = nullptr;
                m_lfile.close();
                openDir(m_pathCurrent);
                emit commandFinish(MT_FTP_DOWNLOAD);
            }
            if( n < 0) {
                handleFtpErrorMessage();
                m_lfile.remove();
            }
        }
        return true;
    }

    virtual bool handleRequest(struct MsgRequest& msg) {
        int type = msg.type;
        QByteArray data = msg.data;
        if(type == MT_FTP_UPLOADNEXT) {
            int n = _runUploading();
            if(n <= 0){
                sftp_close(m_rfile);
                m_rfile = nullptr;
                m_lfile.close();
                m_transfer = eIdle;
                openDir(m_pathCurrent);
                emit commandFinish(MT_FTP_UPLOAD);
            }
            return n >= 0;
        }
        if(type == MT_FTP_ABORT) {
            m_abort = true;
            return true;
        }
        if(m_transfer != eIdle) {
            handleError(tr("the last request is still running, wait for a while."));
            return true;
        }
        if(type == MT_FTP_OPENDIR) {
            QDataStream ds(data);
            QByteArray path;
            ds >> path;
            if(path.isEmpty()) {
                path = ".";
            }
            if(runOpenDir(path) < 0) {
                return false;
            }
        }else if(type == MT_FTP_MKDIR) {
            QDataStream ds(data);
            QByteArray path;
            QByteArray name;
            int mode;
            ds >> path >> name >> mode;
            if(runMkDir(path, name, mode) < 0) {
                return false;
            }
        }else if(type == MT_FTP_RMDIR) {
            QDataStream ds(data);
            QByteArray path;
            QByteArray name;
            ds >> path >> name;
            if(runRmDir(path, name) < 0) {
                return false;
            }
        }else if(type == MT_FTP_UNLINK) {
            QDataStream ds(data);
            QByteArray path;
            QByteArray name;
            ds >> path >> name;
            if(runUnlink(path, name) < 0) {
                return false;
            }
        }else if(type == MT_FTP_UPLOAD) {
            QDataStream ds(data);
            QByteArray local;
            QByteArray remote;
            ds >> local >> remote;
            if(runUpload(local, remote) < 0) {
                return false;
            }
            m_abort = false;
        }else if(type == MT_FTP_DOWNLOAD) {
            QDataStream ds(data);
            QByteArray local;
            QByteArray remote;
            ds >> remote >> local;            
            if(runDownload(remote,local) < 0) {
                return false;
            }
            m_abort = false;
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

    void shellWrite(QWoSshChannel *cli, const QByteArray& data) {
        push(MT_PTYDATA, data, cli);
    }

    void shellSize(QWoSshChannel *cli, int cols, int rows) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << cols << rows;
        push(MT_PTYSIZE, buf, cli);
    }

    void sftpOpenDir(QWoSshChannel *cli, const QString& path) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8();
        push(MT_FTP_OPENDIR, buf, cli);
    }

    void sftpMkDir(QWoSshChannel *cli, const QString& path, const QString& name, int mode) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << name.toUtf8() << mode;
        push(MT_FTP_MKDIR, buf, cli);
    }

    void sftpRmDir(QWoSshChannel *cli, const QString& path, const QString& name) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << name.toUtf8();
        push(MT_FTP_RMDIR, buf, cli);
    }

    void sftpUnlink(QWoSshChannel *cli, const QString &path, const QString &name) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << path.toUtf8() << name.toUtf8();
        push(MT_FTP_UNLINK, buf, cli);
    }

    void sftpDownload(QWoSshChannel *cli, const QString& remote, const QString& local) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << remote.toUtf8() << local.toUtf8();
        push(MT_FTP_DOWNLOAD, buf, cli);
    }

    void sftpUpload(QWoSshChannel *cli, const QString& local, const QString& remote) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << local.toUtf8() << remote.toUtf8();
        push(MT_FTP_UPLOAD, buf, cli);
    }

    void internalUploadNext(QWoSshChannel *cli) {
        push(MT_FTP_UPLOADNEXT, QByteArray(), cli);
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
                        if(msg.type == MT_FTP_UPLOADNEXT) {
                            break;
                        }
                    }
                }else{
                    return -1;
                }
            }
            for(int i = 0; i < MAX_CHANNEL && out[i] != 0; i++) {
                done = true;
                QWoSshChannel *chn = m_all.value(out[i]);
                if(!chn->handleRead()) {
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
    , m_listenSocket(0)
    , m_conn(nullptr)
    , m_connectionState(-1)
{
    QObject::connect(this, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
}

QWoSSHConnection::~QWoSSHConnection()
{
    wait();
}


void QWoSSHConnection::append(QWoSshChannel *cli)
{
    m_conn->append(cli);
}

void QWoSSHConnection::remove(QWoSshChannel *cli)
{
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

void QWoSSHConnection::sftpOpenDir(QWoSshChannel *cli, const QString &path)
{
    if(m_conn) {
        m_conn->sftpOpenDir(cli, path);
    }
}

void QWoSSHConnection::sftpMkDir(QWoSshChannel *cli, const QString &path, const QString &name, int mode)
{
    if(m_conn) {
        m_conn->sftpMkDir(cli, path, name, mode);
    }
}

void QWoSSHConnection::sftpRmDir(QWoSshChannel *cli, const QString &path, const QString &name)
{
    if(m_conn) {
        m_conn->sftpRmDir(cli, path, name);
    }
}

void QWoSSHConnection::sftpUnlink(QWoSshChannel *cli, const QString &path, const QString &name)
{
    if(m_conn) {
        m_conn->sftpUnlink(cli, path, name);
    }
}

void QWoSSHConnection::sftpDownload(QWoSshChannel *cli, const QString &remote, const QString &local)
{
    if(m_conn) {
        m_conn->sftpDownload(cli, remote, local);
    }
}

void QWoSSHConnection::sftpUpload(QWoSshChannel *cli, const QString &local, const QString &remote)
{
    if(m_conn) {
        m_conn->sftpUpload(cli, local, remote);
    }
}

void QWoSSHConnection::sftpAbort(QWoSshChannel *cli)
{
    if(m_conn) {
        m_conn->sftpAbort(cli);
    }
}

void QWoSSHConnection::internalUploadNext(QWoSshChannel *cli)
{
    if(m_conn) {
        m_conn->internalUploadNext(cli);
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
        QSshProxyClient *cli = new QSshProxyClient(ti, this);
        if(!cli->init(m_listenSocket, m_listenPort)) {
            return false;
        }
        QObject::connect(cli, SIGNAL(finished()), this, SLOT(onFinished()));
        QObject::connect(cli, SIGNAL(errorArrived(const QByteArray&)), this, SIGNAL(errorArrived(const QByteArray&)));
        QObject::connect(cli, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SIGNAL(passwordArrived(const QString&,const QByteArray&)));
        QObject::connect(cli, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));
        m_proxys.append(cli);
    }
    QObject::connect(m_conn, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(m_conn, SIGNAL(errorArrived(const QByteArray&)), this, SIGNAL(errorArrived(const QByteArray&)));
    QObject::connect(m_conn, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SIGNAL(passwordArrived(const QString&,const QByteArray&)));
    QObject::connect(m_conn, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SLOT(onInputArrived(const QString&,const QString&,bool)));
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
{
}

QWoSshChannel::~QWoSshChannel()
{
}

bool QWoSshChannel::start(const QString &host, int gid)
{
    QWoSshFactory *factory = QWoSshFactory::instance();
    bool ct = false;
    m_conn = factory->get(gid, &ct);
    if(ct) {
        QObject::connect(m_conn, SIGNAL(errorArrived(const QByteArray&)), this, SIGNAL(errorArrived(const QByteArray&)));
        QObject::connect(m_conn, SIGNAL(passwordArrived(const QString&,const QByteArray&)), this, SIGNAL(passwordArrived(const QString&,const QByteArray&)));
        QObject::connect(m_conn, SIGNAL(inputArrived(const QString&,const QString&,bool)), this, SIGNAL(inputArrived(const QString&,const QString&,bool)));
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

void QWoSshChannel::stop()
{
    if(m_conn){
        m_conn->remove(this);
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

void QWoSshFtp::openDir(const QString &path)
{
    if(path.isEmpty()) {
        return;
    }
    if(m_conn) {
        m_conn->sftpOpenDir(this, path);
    }
}

void QWoSshFtp::mkDir(const QString &path, const QString &name, int mode)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpMkDir(this, tmp, name, mode);
    }
}

void QWoSshFtp::rmDir(const QString &path, const QString &name)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpRmDir(this, tmp, name);
    }
}

void QWoSshFtp::unlink(const QString &path, const QString &name)
{
    QString tmp = QDir::cleanPath(path);
    if(m_conn) {
        m_conn->sftpUnlink(this, tmp, name);
    }
}

void QWoSshFtp::download(const QString &remote, const QString &local)
{
    QString path = local;
    if(local.startsWith("file://")) {
        QUrl url(local);
        path = url.toLocalFile();
    }
    if(m_conn) {
        m_conn->sftpDownload(this, remote, path);
    }
}

void QWoSshFtp::upload(const QString &local, const QString &remote)
{
    QString path = local;
    if(local.startsWith("file://")) {
        QUrl url(local);
        path = url.toLocalFile();
    }
    if(m_conn) {
        m_conn->sftpUpload(this, path, remote);
    }
}

void QWoSshFtp::abort()
{
    if(m_conn) {
        m_conn->sftpAbort(this);
    }
}

void QWoSshFtp::onResponse(int type, const QVariant &data)
{
    if(type == FTP_TYPE_OPENDIR) {
        QVariantMap md = data.toMap();
        QString path = md.value("path").toString();
        QVariantList ls = md.value("list").toList();
        emit dirOpen(path, ls);
    }
}

void QWoSshFtp::init()
{
    QObject::connect(this, SIGNAL(response(int,QVariant)), this, SLOT(onResponse(int,QVariant)));
    QObject::connect(m_conn, SIGNAL(commandStart(int)), this, SIGNAL(commandStart(int)));
    QObject::connect(m_conn, SIGNAL(commandFinish(int)), this, SIGNAL(commandFinish(int)));
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
