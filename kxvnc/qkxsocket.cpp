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

#include "qkxsocket.h"
#include "qkxutils.h"
#include "qkxopensslthreadcryptosafety.h"

#include <QDateTime>
#include <QtEndian>
#include <QDebug>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QSslSocket>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fcntl.h>
#define myclosesocket  closesocket
typedef SOCKET socket_t;
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
typedef int socket_t;
#define myclosesocket(x)    close(x)
#endif

#define MAX_BUFFER  (1024 * 1024)

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

static int sock_read_ready(SSL *ssl, uint32_t ms)
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(SSL_get_fd(ssl), &fds);

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return select (SSL_get_fd(ssl) + 1, &fds, NULL, NULL, &tv);
}

static int wait_for_data(SSL *ssl, int ret, int timeout)
{
    int retval = 1;
    int err = SSL_get_error(ssl, ret);

    switch(err) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
        ret = sock_read_ready(ssl, timeout*1000);
        if (ret == -1) {
            retval = 2;
        }
        break;
    default:
        retval = 3;
        long verify_res = SSL_get_verify_result(ssl);
        if (verify_res != X509_V_OK){
            qDebug() << QString("Could not verify server certificate: %1.").arg(X509_verify_cert_error_string(verify_res));
        }
        break;
    }
    ERR_clear_error();
    return retval;
}

QTLSAuth::QTLSAuth(int sockFd, QObject *parent)
    : QObject(parent)
    , m_socket(sockFd) {

}

QTLSAuth::~QTLSAuth(){

}

int QTLSAuth::sslErrorNo(int err)
{
    switch (err) {
    case SSL_ERROR_NONE:
        return 0;
    case SSL_ERROR_ZERO_RETURN:
        return EINVAL;
    case SSL_ERROR_WANT_READ:   /* non-fatal; retry */
    case SSL_ERROR_WANT_WRITE:  /* non-fatal; retry */
        return EAGAIN;
    case SSL_ERROR_SYSCALL:
        return EINTR;
    case SSL_ERROR_SSL:
        return EINTR;
    default:
        return EINTR;
    }
}


int QTLSAuth::readFromTLS(char *buf, int len) {
    int left = len;
    while(left > 0) {
        int n = SSL_read (m_ssl, buf, left);
        if(n < 0) {
            int err = SSL_get_error(m_ssl, n);
            if(QKxUtils::isAgain(sslErrorNo(err))) {
                return len - left;
            }
            return n;
        }
        if(n == 0) {
            return len - left;
        }
        buf += n;
        left -= n;
    }
    return len;
}

int QTLSAuth::writeToTLS(const char *buf, int len) {
    int left = len;
    while(left > 0) {
        int n = SSL_write(m_ssl, buf, left);
        if(n < 0) {
            int err = SSL_get_error(m_ssl, n);
            if(QKxUtils::isAgain(sslErrorNo(err))) {
                return len - left;
            }
            return n;
        }
        if(n == 0) {
            return len - left;
        }
        buf += n;
        left -= n;
    }
    return len;
}

class QTLSAnonAuth : public QTLSAuth
{
public:
    explicit QTLSAnonAuth(int sockFd, QObject *parent=nullptr)
        : QTLSAuth(sockFd, parent){
        m_ssl = openSslConnection(sockFd);
    }
private:
    SSL *openSslConnection(int sockFd) {
        SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
        if (ssl_ctx == nullptr) {
            qDebug() << "Could not create new SSL context.";
            return nullptr;
        }
        // https://www.openssl.org/docs/manmaster/man1/openssl-ciphers.html
        SSL_CTX_set_cipher_list(ssl_ctx, "aNULL");
        SSL_CTX_set_security_level(ssl_ctx, 0);
        SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
        SSL *ssl = SSL_new(ssl_ctx);
        if(ssl == nullptr) {
            SSL_CTX_free(ssl_ctx);
            return nullptr;
        }
        SSL_set_fd(ssl, sockFd);
        SSL_CTX_set_app_data (ssl_ctx, this);
        int n = 0;
        int finished = 0;
        do {
            n = SSL_connect(ssl);
            if (n != 1) {
                if (wait_for_data(ssl, n, 1) != 1)  {
                    finished = 1;
                    SSL_shutdown(ssl);
                }
            }
        }while( n != 1 && finished != 1 );
        return ssl;
    }
};


QKxTcpSocket::QKxTcpSocket(QObject *parent)
    : QKxReader(parent)
{
    m_socket = -1;
    m_ipos = 0;
    m_buf.reserve(MAX_BUFFER);
}

QKxTcpSocket::~QKxTcpSocket()
{
    if(m_socket > 0) {
        myclosesocket(m_socket);
    }
    //delete m_auth;
}

bool QKxTcpSocket::connect(const char *host, int port)
{
    struct addrinfo *ai = nullptr;
    struct addrinfo *itr = nullptr;
    int err = QKxUtils::getAddrInfos(host, port, &ai);
    if(err != 0) {
        return false;
    }
    bool ok = false;
    for (itr = ai; itr != nullptr; itr = itr->ai_next) {
        /* create socket */
        socket_t fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);
        err = ::connect(fd, itr->ai_addr, itr->ai_addrlen);
        if(err != 0){
            myclosesocket(fd);
            continue;
        }
        QKxUtils::setSocketKeepAlive(fd, true);
        QKxUtils::setSocketNoDelay(fd, true);
        QKxUtils::setSocketNonBlock(fd, true);
        QKxUtils::setSocketRecvBufferSize(fd, 1024 * 1024 * 8);
        QKxUtils::setSocketSendBufferSize(fd, 1024 * 1024 * 8);
        m_socket = fd;
        ok = true;
        break;
    }
    QKxUtils::freeAddrInfos(ai);
    return ok;
}


int QKxTcpSocket::_waitRead(char *buf, int len, int timeout)
{
    if(m_buf.length() >= (len + m_ipos)) {
        char *psrc = (char*)m_buf.data() + m_ipos;
        memcpy(buf, psrc, len);
        m_ipos += len;
        return len;
    }
    m_buf.remove(0, m_ipos);
    m_ipos = 0;
    if(_waitBuffer(len, timeout) < len) {
        return -1;
    }
    return _waitRead(buf, len, timeout);
}

int QKxTcpSocket::_waitWrite(char *buf, int len, int timeout)
{
    qint64 tmStart = QDateTime::currentMSecsSinceEpoch();
    int nWrite = m_auth != nullptr ? m_auth->writeToTLS(buf, len) : QKxUtils::xSend(m_socket, buf, len);
    if(nWrite < 0) {
        return nWrite;
    }
    while(nWrite < len) {
        qint64 tmNow = QDateTime::currentMSecsSinceEpoch();
        qint64 tmLeft = (qint64)timeout - tmNow + tmStart;
        if(tmLeft < 0) {
            return -1;
        }
        int sec = tmLeft / 1000;
        int usec = (tmLeft % 1000) * 1000;
        timeval tm={sec, usec};
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(m_socket, &wfds);
        int n = select(m_socket+1, nullptr, &wfds, nullptr, &tm);
        if(n < 0) {
            return n;
        }
        if(n == 0) {
            continue;
        }
        if(FD_ISSET(m_socket, &wfds)) {
            int n = m_auth != nullptr ? m_auth->writeToTLS(buf + nWrite , len - nWrite) : QKxUtils::xSend(m_socket, buf + nWrite , len - nWrite);
            if(n < 0) {
                return n;
            }
            nWrite += n;
        }
    }
    return nWrite;
}

int QKxTcpSocket::_waitBuffer(int len, int timeout)
{
    if(len > m_buf.capacity()) {
        m_buf.reserve(len * 1.5);
    }
    qint64 tmStart = QDateTime::currentMSecsSinceEpoch();
    if(_readAll() >= len) {
        return m_buf.length();
    }
    while(1) {
        qint64 tmNow = QDateTime::currentMSecsSinceEpoch();
        qint64 tmLeft = (qint64)timeout - tmNow + tmStart;
        if(tmLeft < 0) {
            return -1;
        }
        int sec = tmLeft / 1000;
        int usec = (tmLeft % 1000) * 1000;
        timeval tm={sec, usec};
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_socket, &rfds);
        int n = select(m_socket+1, &rfds, nullptr, nullptr, &tm);
        if(n < 0) {
            return n;
        }
        if(n == 0) {
            return -99;
        }
        if(_readAll() >= len) {
            return m_buf.length();
        }
    }
}

int QKxTcpSocket::handle()
{
    return m_socket;
}

bool QKxTcpSocket::makeAnonTLS()
{
    m_auth = new QTLSAnonAuth(m_socket, this);
    return true;
}

bool QKxTcpSocket::waitRead(char *buf, int len, int timeout)
{
    return _waitRead(buf, len, timeout) == len;
}

bool QKxTcpSocket::waitWrite(char *buf, int len, int timeout)
{
    return _waitWrite(buf, len, timeout) == len;
}

bool QKxTcpSocket::waitWrite(const QByteArray &buf, int timeout)
{
    return waitWrite((char*)buf.data(), buf.length(), timeout);
}

bool QKxTcpSocket::isEmpty()
{
    return m_buf.length() == m_ipos;
}

QByteArray QKxTcpSocket::readArray(int timeout)
{
    QByteArray buf;
    int len = readInt32(timeout);
    buf.resize(len);
    if(!waitRead(buf.data(), len)) {
        throw -1;
    }
    return buf;
}

qint32 QKxTcpSocket::readInt32(int timeout)
{
    qint32 v;
    if(!waitRead((char*)&v, 4, timeout)) {
        throw -1;
    }
    return qFromBigEndian<qint32>(v);
}

quint32 QKxTcpSocket::readUint32(int timeout)
{
    quint32 v;
    if(!waitRead((char*)&v, 4, timeout)) {
        throw -1;
    }
    return qFromBigEndian<quint32>(v);
}

qint16 QKxTcpSocket::readInt16(int timeout)
{
    qint16 v;
    if(!waitRead((char*)&v, 2, timeout)) {
        throw -1;
    }
    return qFromBigEndian<qint16>(v);
}

quint16 QKxTcpSocket::readUint16(int timeout)
{
    quint16 v;
    if(!waitRead((char*)&v, 2, timeout)) {
        throw -1;
    }
    return qFromBigEndian<quint16>(v);
}

qint8 QKxTcpSocket::readInt8(int timeout)
{
    qint8 v;
    if(!waitRead((char*)&v, 1, timeout)) {
        throw -1;
    }
    return v;
}

quint8 QKxTcpSocket::readUint8(int timeout)
{
    quint8 v;
    if(!waitRead((char*)&v, 1, timeout)) {
        throw -1;
    }
    return v;
}

quint32 QKxTcpSocket::readRgb32(int timeout)
{
    quint32 v;
    if(!waitRead((char*)&v, 4, timeout)) {
        throw -1;
    }
    return v;
}

quint32 QKxTcpSocket::readRgb24(int timeout)
{
    quint8 v[3];
    if(!waitRead((char*)v, 3, timeout)) {
        throw -1;
    }
    quint32 r = v[0];
    quint32 g = v[1];
    quint32 b = v[2];
    return (b << 16) | (g << 8) | r;
}

quint16 QKxTcpSocket::readRgb16(int timeout)
{
    quint16 v;
    if(!waitRead((char*)&v, 2, timeout)) {
        throw -1;
    }
    return v;
}

quint8 QKxTcpSocket::readRgb8(int timeout)
{
    quint8 v;
    if(!waitRead((char*)&v, 1, timeout)) {
        throw -1;
    }
    return v;
}

void QKxTcpSocket::readSkip(int len, int timeout)
{
    if(m_buf.length() >= (len + m_ipos)) {
        m_ipos += len;
        return;
    }
    m_buf.remove(0, m_ipos);
    m_ipos = 0;
    if(_waitBuffer(len, timeout) < 0) {
        throw -1;
    }
    m_ipos += len;
}

char *QKxTcpSocket::current(int len, int timeout)
{
    if(m_buf.length() >= (len + m_ipos)) {
        return m_buf.data() + m_ipos;
    }
    m_buf.remove(0, m_ipos);
    m_ipos = 0;
    if(_waitBuffer(len, timeout) < 0) {
        throw -1;
    }
    return m_buf.data();
}

int QKxTcpSocket::_readAll()
{
    char *buf = m_buf.data() + m_buf.length();
    int total = m_buf.length();
    while(1) {
        int cnt = (m_auth != nullptr ? m_auth->readFromTLS(buf, m_buf.capacity() - total - 128) : QKxUtils::xRecv(m_socket, buf, m_buf.capacity() - total - 128));
        if(cnt <= 0) {
            m_buf.resize(total);
            return m_buf.length();
        }
        buf += cnt;
        total += cnt;
    }
}
