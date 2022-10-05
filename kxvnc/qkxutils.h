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

#ifndef QKXUTILS_H
#define QKXUTILS_H

#include <QVariant>
#include <QString>

struct addrinfo;

class QKxUtils
{
public:
    /*socket*/
    static int getAddrInfos(const char *host, int port, struct addrinfo **ai);
    static void freeAddrInfos(struct addrinfo* addr);
    static bool isAgain(int err);
    static void setSocketNonBlock(int sock, bool on);
    static void setSocketNoDelay(int sock, bool on);
    static void setSocketReusable(int sock, bool on);
    static void setSocketKeepAlive(int sock, bool on);
    static void setSocketSendBufferSize(int sock, int cnt);
    static void setSocketRecvBufferSize(int sock, int cnt);
    static bool createPair(int server, ushort port, int fd[2]);
    static bool createPair2(ushort basePort, int fd[2]);

    static ushort listenLocal(int server, ushort basePort);
    static int socketError();
    static int xRecv(int sock, char *buf, int len, int flag=0);
    static int xSend(int sock, char *buf, int len, int flag=0);
};

#endif // QKXUTILS_H
