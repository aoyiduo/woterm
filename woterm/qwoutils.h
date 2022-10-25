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

#include <QVariant>
#include <QString>


class QLayout;
struct addrinfo;

class QWoUtils
{
public:
    static void setLayoutVisible(QLayout* layout, bool visible);
    static QString qVariantToBase64(const QVariant& v);
    static QVariant qBase64ToVariant(const QString& v);
    static QStringList parseCombinedArgString(const QString &program);
    static QByteArray rc4(const QByteArray& data, const QByteArray& key);
    static QString nameToPath(const QString &name);
    static QString pathToName(const QString &path);
    static int versionToLong(const QString& ver);
    static QByteArray toWotermStream(const QByteArray &data);
    static QByteArray fromWotermStream(const QByteArray& data);
    static int hasDesktopShortcut(bool everyone);
    static int createDesktopShortcut(bool everyone, const QString& name, const QString& desc, const QString& iconPath);
    static void removeDesktopShortcut(bool everyone);
    static QByteArray filePermissionToText(int type, int flag);


    /*socket*/
    static int getAddrInfos(const char *host, int port, struct addrinfo **ai);
    static void freeAddrInfos(struct addrinfo* addr);
    static bool isAgain(int err);
    static void setSocketNonBlock(int sock, bool on);
    static void setSocketNoDelay(int sock, bool on);
    static void setSocketReusable(int sock, bool on);
    static void setSocketKeepAlive(int sock, bool on);
    static bool createPair(int server, ushort port, int fd[2]);
    static bool createPair2(ushort basePort, int fd[2]);

    static ushort listenLocal(int server, ushort basePort);
    static int socketError();
    static int xRecv(int sock, char *buf, int len, int flag=0);
    static int xSend(int sock, char *buf, int len, int flag=0);

    static int gid();

    static bool isRootUser();
    static QString loginUser();

    static bool openself(const QString& type, const QString& target, bool pkexec=false);

    static int parseVersion(const QString& ver);
};
