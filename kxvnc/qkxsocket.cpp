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

#include "qkxsocket.h"
#include "qkxutils.h"

#include <QDateTime>
#include <QtEndian>
#include <QDebug>
#include <QMutex>
#include <QVector>

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

#ifdef HAVE_TLS

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rand.h>

bool gTLSInit() {
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    return true;
}

QTLSAuth::QTLSAuth(QObject *parent)
    : QObject(parent)
{
    static bool btls = gTLSInit();
}

QTLSAuth::~QTLSAuth()
{

}


class QPowerTLSAuth : public QTLSAuth
{
private:
    int m_fd;
public:
    explicit QPowerTLSAuth(int fd, QObject *parent=nullptr)
        : QTLSAuth(parent)
        , m_fd(fd) {
    }

    ~QPowerTLSAuth(){

    }

    bool init() {
        return _init();
    }

protected:
    int sock_read_ready(SSL *ssl, uint32_t ms) {
        int r = 0;
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(SSL_get_fd(ssl), &fds);
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        r = select (SSL_get_fd(ssl) + 1, &fds, NULL, NULL, &tv);
        return r;
    }

    int waitForData(SSL *ssl, int ret, int timeout) {
        int retval = 1;
        int err = SSL_get_error(ssl, ret);
        if(err == SSL_ERROR_WANT_READ || err ==SSL_ERROR_WANT_WRITE) {
            int ret = sock_read_ready(ssl, timeout*1000);
            if (ret == -1) {
                retval = 2;
            }
        }else{
            retval = 3;
            long verify_res = SSL_get_verify_result(ssl);
            if (verify_res != X509_V_OK){
                qDebug() << "Could not verify server certificate:" << X509_verify_cert_error_string(verify_res);
            }
        }
        ERR_clear_error();
        return retval;
    }

    bool _init() {
        SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
        if (ssl_ctx == nullptr) {
            return false;
        }
        X509_VERIFY_PARAM *param = X509_VERIFY_PARAM_new();
        SSL_CTX_set_cipher_list(ssl_ctx, "ADH");
        SSL *ssl = SSL_new (ssl_ctx);
        SSL_set_fd (ssl, m_fd);
        SSL_CTX_set_app_data (ssl_ctx, this);

        int n = 0;
        int finished = 0;
        do{
            int n = SSL_connect(ssl);
            if(n != 1) {
                if(waitForData(ssl, n, 1) != 1) {
                    finished = 1;
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    SSL_CTX_free(ssl_ctx);
                    return false;
                }
            }
        }while(n != 1);
        X509_VERIFY_PARAM_free(param);
        return true;
    }

    virtual bool handleAnonTLSAuth() {

        return true;
    }

    virtual bool handleVeNCryptAuth() {
        return true;
    }

    virtual int readFromTLS(char *out, unsigned int n){
        return 0;
    }

    virtual int writeToTLS(const char *buf, unsigned int n) {
        return 0;
    }

    virtual void freeTLS(){

    }
};

#endif

QKxTcpSocket::QKxTcpSocket()
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
    int nWrite = QKxUtils::xSend(m_socket, buf, len);
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
            int n = QKxUtils::xSend(m_socket, buf + nWrite , len - nWrite);
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

bool QKxTcpSocket::makeTLS()
{
//    if(m_auth) {
//        return true;
//    }
//    QPowerTLSAuth *auth = new QPowerTLSAuth(m_socket);
//    m_auth = auth;
//    return auth->init();
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
        int cnt = QKxUtils::xRecv(m_socket, buf, m_buf.capacity() - total - 128);
        if(cnt <= 0) {
            m_buf.resize(total);
            return m_buf.length();
        }
        buf += cnt;
        total += cnt;
    }
}
