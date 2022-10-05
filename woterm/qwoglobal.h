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

#include <QObject>
#include <QUuid>
#include <QVariantMap>

enum EHostType{
    SshWithSftp = 1,
    SftpOnly,
    SshBase,
    Telnet,
    RLogin,
    SerialPort,
    Mstsc,
    Vnc,
    All
};

#define EOT_SSH         (1)
#define EOT_SFTP        (2)
#define EOT_TELNET      (3)
#define EOT_RLOGIN      (4)
#define EOT_SERIALPORT  (5)
#define EOT_MSTSC       (6)
#define EOT_VNC         (7)


struct HostInfo{
    EHostType type;
    QString name;
    QString host;
    int port;
    QString user;
    QString password;
    QString identityFile;
    QString scriptFile;
    QString script;
    QString proxyJump;
    QString memo;
    QString property;
    QString group;
    QString baudRate;
    QString dataBits;
    QString parity;
    QString stopBits;
    QString flowContrl;

    inline bool isValid() {
        return !name.isEmpty();
    }

    inline bool hasProxyJump() {
        return !proxyJump.isEmpty();
    }

    inline bool hasIdentify() {
        return !identityFile.isEmpty();
    }
};

struct HistoryCommand {
    QString cmd;
    QString path;

    bool operator==(const HistoryCommand& other) {
        return other.cmd == cmd;
    }
};

typedef struct{
    QString path;
    QString name;
    QString fingureprint;
    QString key;
    QString type;
} IdentifyInfo;


#define FI_ReadOwner 0x4000
#define FI_WriteOwner 0x2000
#define FI_ExeOwner 0x1000
#define FI_ReadUser 0x0400
#define FI_WriteUser 0x0200
#define FI_ExeUser 0x0100
#define FI_ReadGroup 0x0040
#define FI_WriteGroup 0x0020
#define FI_ExeGroup 0x0010
#define FI_ReadOther 0x0004
#define FI_WriteOther 0x0002
#define FI_ExeOther 0x0001
typedef struct {
    QString longName;
    QString name;
    QString type;
    QString owner;
    QString group;
    QString size;
    QString date;
    QString label;
} FileInfo;


Q_DECLARE_METATYPE(HostInfo)
Q_DECLARE_METATYPE(HistoryCommand)
Q_DECLARE_METATYPE(FileInfo)

#define DEFAULT_HISTORY_LINE_LENGTH  (1000)


#define PERFORMANCE_EXECUTE(x) \
{   \
    QTime t1 = QTime::currentTime(); \
    {x} \
    QTime t2 = QTime::currentTime(); \
    qDebug() << "use:" << t1.msecsTo(t2); \
}
