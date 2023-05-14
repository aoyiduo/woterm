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
#define EOT_PTY         (8)

#define ROLE_INDEX              (Qt::UserRole+1)
#define ROLE_HOSTINFO           (Qt::UserRole+2)
#define ROLE_REFILTER           (Qt::UserRole+3)
#define ROLE_FRIENDLY_NAME      (Qt::UserRole+4)
#define ROLE_FILEINFO           (Qt::UserRole+5)
#define ROLE_GROUP              (Qt::UserRole+6)
#define ROLE_GROUP_STATE        (Qt::UserRole+7)
#define ROLE_TASKINFO           (Qt::UserRole+8)
#define ROLE_CUSTOM_FONT        (Qt::UserRole+9)
#define ROLE_ICON_URL           (Qt::UserRole+10)
#define ROLE_SHORT_STRING       (Qt::UserRole+11)
#define ROLE_FULL_STRING        (Qt::UserRole+12)
#define ROLE_GROUP_NAME         (Qt::UserRole+13)
#define ROLE_TYPE               (Qt::UserRole+14)
#define ROLE_FILENAME           (Qt::UserRole+15)
#define ROLE_ISDIR              (Qt::UserRole+16)
#define ROLE_FILESIZE           (Qt::UserRole+17)
#define ROLE_ISROOT             (Qt::UserRole+18)
#define ROLE_FILEPATH           (Qt::UserRole+19)
#define ROLE_FILEURL            (Qt::UserRole+20)
#define ROLE_SELECTED           (Qt::UserRole+21)
#define ROLE_PATH_LOCAL         (Qt::UserRole+22)
#define ROLE_PATH_REMOTE        (Qt::UserRole+23)
#define ROLE_MENUID             (Qt::UserRole+24)


struct GroupInfo {
    QString name;
    int order;

    bool operator < (const GroupInfo& b) const {
        return order < b.order;
    }
};

struct HostInfo{
    EHostType type;
    QString name;
    QString host;
    int port;
    QString user;
    QString password;
    QString identityFile;
    QString identityContent;
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
    QString flowControl;

    bool operator < (const HostInfo& hi) const {
        return name < hi.name;
    }

    inline bool isValid() const {
        return !name.isEmpty();
    }

    inline bool hasProxyJump() const {
        return !proxyJump.isEmpty();
    }

    inline bool hasIdentify() const {
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
    QString name;
    QString fingureprint;
    QString type;
    QByteArray prvKey;
    QByteArray pubKey;
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
struct FileInfo {
    QString longName;
    QString name;
    QString type;
    QString owner;
    QString group;
    qint64 size;
    QString date;
    QString label;
    QString permission;

    // add for multiple selection.
    bool selected; // temp record select or not.

    FileInfo() {
        selected = false;
    }

    bool isDir() const {
        return type == 'd';
    }
    bool isFile() const {
        return type == '-';
    }
    bool isLink() const {
        return type == 'l';
    }
};

enum ETState {
    ETS_None = 1,
    ETS_Ready,
    ETS_Finished,
    ETS_Error
};
struct TaskInfo {
    int taskId;
    QString local;
    QString remote;
    int textWidth;
    bool isDir;
    bool isDown;
    bool isAppend;
    ETState state;
    int tryCount;
    int fileCount;

    TaskInfo() {
        state = ETS_None;
        taskId = -1;
        isDir = isDown = isAppend = false;
        textWidth = 0;
        tryCount = 0;
        fileCount = 0;
    }

    bool isValid() const {
        return taskId > 0;
    }

    void clear() {
        isDir = isDown = isAppend = false;
        state = ETS_None;
        textWidth = 0;
        taskId = 0;
        tryCount = 0;
        fileCount = 0;
        local.clear();
        remote.clear();
    }
};

#define DB_MERGE_ACTION_ADD         (1)
#define DB_MERGE_ACTION_REMOVE      (2)
#define DB_MERGE_ACTION_REPLACE     (3)
struct MergeInfo {
    QList<QVariantMap> same; //
    QList<QVariantMap> lhave; // only local has but remote.
    QList<QVariantMap> rhave; // only remote has but local.
    QList<QVariantMap> replace; // different, can replace by remote.
    QList<QVariantMap> remove; // only hidden.

    void clear() {
        same.clear();
        lhave.clear();
        rhave.clear();
        replace.clear();
        remove.clear();
    }

    QList<QVariantMap> result(bool isFull) const {
        if(isFull) {
            return rhave + lhave + replace + same;
        }
        return rhave + lhave + replace;
    }
    QString resultInformation() const {
        if(rhave.isEmpty() && lhave.isEmpty() && replace.isEmpty()) {
            return QObject::tr("The current records are exactly the same.");
        }
        QStringList summarys;
        if(!rhave.isEmpty()) {
            summarys.append(QObject::tr("Number of records that can be added: %1").arg(rhave.length()));
        }
        if(!lhave.isEmpty()) {
            summarys.append(QObject::tr("Number of records that can be removed: %1").arg(lhave.length()));
        }
        if(!replace.isEmpty()) {
            summarys.append(QObject::tr("Number of records that can be replaced: %1").arg(replace.length()));
        }
        if(!same.isEmpty()) {
            summarys.append(QObject::tr("Number of records that are the same: %1").arg(same.length()));
        }
        return summarys.join("\r\n");
    }
};

Q_DECLARE_METATYPE(MergeInfo)
Q_DECLARE_METATYPE(TaskInfo)
Q_DECLARE_METATYPE(GroupInfo)
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
