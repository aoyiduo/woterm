﻿/*******************************************************************************************
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

#include <QObject>
#include <QVariant>
#include <QString>


class QQmlEngine;
class QWidget;
class QLayout;
struct addrinfo;


class QWoUtils : public QObject
{
    Q_OBJECT
public:
    explicit QWoUtils(QObject* parent = nullptr);
    ~QWoUtils();

    static bool isDebugVersion();

    static void injectJS(QQmlEngine *engine, QStringList files);
    static void setLayoutVisible(QLayout* layout, bool visible);
    static QString qVariantToBase64(const QVariant& v);
    static QVariant qBase64ToVariant(const QString& v);
    static QMap<QString, QVariantMap> qBase64ToMoreConfigure(const QString& v);
    static QString qBase64FromMoreConfigure(const QMap<QString, QVariantMap>& v);
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
    static QString getDefaultGateway();

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
    static bool createPair3(int fd[]);

    static ushort listenLocal(int server, ushort basePort);
    static int socketError();
    static int xRecv(int sock, char *buf, int len, int flag=0);
    static int xSend(int sock, char *buf, int len, int flag=0);

    static int gid();

    static bool isRootUser();
    static bool hasUnprivilegedPortPermission();
    static QString loginUser();

    static bool openself(const QString& type, const QString& target, bool pkexec=false);
    static bool runAsRoot(const QString& cmd);

    static int parseVersion(const QString& ver);

    /*encrypt*/
    static QByteArray aesOfb128Encrypt(const QByteArray &all, const QByteArray &pass);
    static QByteArray aesOfb128Decrypt(const QByteArray &all, const QByteArray &pass);
    static QByteArray aesEncrypt(const QByteArray& all, const QByteArray &pass="WoTerm@2022-11-6");
    static QByteArray aesDecrypt(const QByteArray& all, const QByteArray &pass="WoTerm@2022-11-6");

    /*version*/
    static bool isUltimateVersion(QWidget *parent);
    static QString getPassword(QWidget *parent, const QString& label);

    /* remove directory */
    static bool removeDirectory(const QString& path);
    static void copyDirectory(const QString& src, const QString& dst);

    static QString findShellPath();

    Q_INVOKABLE static QString qmlCleanPath(const QString& path);

    static void setVisibleOnDock(bool yes);
};
