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

#include "qkxutils.h"

#include <QLibrary>
#include <QDateTime>
#include <QImage>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fcntl.h>
#define myclosesocket  closesocket
typedef int socklen_t;

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

#include <QDebug>

static ulong LOCAL_IP = inet_addr("127.0.0.1");

int QKxUtils::getAddrInfos(const char *host, int port, addrinfo **ai)
{
    const char *service = nullptr;
    struct addrinfo hints;
    char s_port[10];

    memset(&hints, 0, sizeof(hints));

    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (port == 0) {
        hints.ai_flags = AI_PASSIVE;
    } else {
        qsnprintf(s_port, sizeof(s_port), "%hu", (unsigned short)port);
        service = s_port;
#ifdef AI_NUMERICSERV
        hints.ai_flags = AI_NUMERICSERV;
#endif
    }

    return getaddrinfo(host, service, &hints, ai);
}

void QKxUtils::freeAddrInfos(addrinfo *addr)
{
    freeaddrinfo(addr);
}

bool QKxUtils::isAgain(int err)
{
#ifdef Q_OS_WIN
    return err == EAGAIN || err == EWOULDBLOCK || err == WSAEWOULDBLOCK;
#else
    return err == EAGAIN;
#endif
}

void QKxUtils::setSocketNonBlock(int sock, bool on)
{
#ifdef Q_OS_WIN
    ulong nonblocking = on ? 1 : 0;
    ioctlsocket(sock, FIONBIO, &nonblocking);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if(on) {
        fcntl(sock, F_SETFL, flags|O_NONBLOCK);
    }else{
        fcntl(sock, F_SETFL, flags&~O_NONBLOCK);
    }
#endif
}

void QKxUtils::setSocketNoDelay(int sock, bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));
}

void QKxUtils::setSocketReusable(int sock, bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
}

void QKxUtils::setSocketKeepAlive(int sock, bool on)
{
    int flag = on ? 1 : 0;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(flag));
}

void QKxUtils::setSocketSendBufferSize(int sock, int cnt)
{
    int cntOrg = 0;
    int cntNew = 0;
    socklen_t szCnt = sizeof(cntOrg);
    int err1 = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&cntOrg, &szCnt);
    int err2 = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&cnt, sizeof(cnt));
    int err3 = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&cntNew, &szCnt);
    qDebug() << err1 << err2 << err3 << cntOrg << cnt << cntNew;
}

void QKxUtils::setSocketRecvBufferSize(int sock, int cnt)
{
    int cntOrg = 0;
    int cntNew = 0;
    socklen_t szCnt = sizeof(cntOrg);
    int err1 = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&cntOrg, &szCnt);
    int err2 = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&cnt, sizeof(cnt));
    int err3 = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&cntNew, &szCnt);
    qDebug() << err1 << err2 << err3 << cntOrg << cnt << cntNew;
}

bool QKxUtils::createPair(int server, ushort port, int fd[])
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = LOCAL_IP;
    addr.sin_port = htons(port);
    int fd1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(::connect(fd1, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        myclosesocket(fd1);
        return false;
    }
    int fd2 = accept(int(server), nullptr, nullptr);
    fd[0] = int(fd1);
    fd[1] = int(fd2);
    return true;
}

bool QKxUtils::createPair2(ushort basePort, int fd[])
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(bind(server, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        myclosesocket(server);
        return false;
    }
    if(listen(server, 5) == -1) {
        myclosesocket(server);
        return false;
    }
    int asize = sizeof(addr);
    ::getsockname(server, (struct sockaddr*)&addr, (socklen_t*)&asize);
    quint16 port = htons(addr.sin_port);
    qInfo() << "QKxUtils::createPair listen:" << port;
    int fd1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(::connect(fd1, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        myclosesocket(fd1);
        myclosesocket(server);
        return false;
    }
    int fd2 = accept(server, nullptr, nullptr);
    fd[0] = int(fd1);
    fd[1] = int(fd2);
    myclosesocket(server);
    return true;

}

ushort QKxUtils::listenLocal(int server, ushort basePort)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = LOCAL_IP;
    for(ushort i = 0; i < 100; i++){
        addr.sin_port = htons(basePort + i);
        if(bind(server, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
            continue;
        }
        if(listen(server, 5) == -1) {
            return 0;
        }
        return basePort + i;
    }
    return 0;
}

int QKxUtils::socketError()
{
#ifdef Q_OS_WIN
    int err = WSAGetLastError();
    return err;
#else
    return errno;
#endif
}

int QKxUtils::xRecv(int sock, char *buf, int len, int flag)
{
    int left = len;
    while(left > 0) {
        int n = ::recv(sock, buf, left, flag);
        if(n < 0) {
            if(QKxUtils::isAgain(socketError())) {
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

int QKxUtils::xSend(int sock, char *buf, int len, int flag)
{
    int left = len;
    while(left > 0) {
        int n = ::send(sock, buf, left, flag);
        if(n < 0) {
            if(QKxUtils::isAgain(socketError())) {
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
