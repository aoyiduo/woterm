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

#include "qwoglobal.h"

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QUrl>

class QBuffer;
class QTimer;

class QWoSshConf : public QObject
{
    Q_OBJECT
public:
    explicit QWoSshConf(const QString& dbFile, QObject *parent = nullptr);
    static QWoSshConf* instance();
    static bool databaseValid(const QString& dbFile);
    /* groud name */
    QList<GroupInfo> groupList() const;
    QStringList groupNameList() const;
    Q_INVOKABLE QStringList qmlGroupNameList() const;
    QStringList tableList() const;

    bool renameGroup(const QString& nameNew, const QString& nameOld);
    bool updateGroup(const QString& name, int order = 0);
    bool removeGroup(const QString& name);


    Q_INVOKABLE bool restoreByUrl(const QUrl& url);
    bool restore(const QString& dbFile);
    bool backup(const QString& dbFile);
    bool refresh();
    bool exportToFile(const QString& path);
    bool removeServer(const QString& name);
    Q_INVOKABLE bool qmlRemoveServer(const QString& name);
    Q_INVOKABLE void qmlClear();
    bool removeServerByGroup(const QString& name);
    bool renameServerGroup(const QString& nameNew, const QString& nameOld);
    bool modify(const HostInfo& hi);
    bool append(const HostInfo& hi);
    bool modifyOrAppend(const HostInfo& hi);
    Q_INVOKABLE bool qmlModifyOrAppend(const QVariant& v);
    HostInfo find(const QString&name) const;
    Q_INVOKABLE QVariant qmlFind(const QString& name) const;
    Q_INVOKABLE QVariant qmlDefault() const;
    bool find(const QString &name, HostInfo* pinfo) const;


    void resetAllProperty(const QString& v);
    Q_INVOKABLE bool exists(const QString& name) const;
    void updatePassword(const QString& name, const QString& password);

    QList<HostInfo> hostList(EHostType type = All) const;
    QStringList hostNameList(EHostType type = All) const;
    QList<HostInfo> proxyJumpers(const QString& name, int max=2) const;
    Q_INVOKABLE QStringList qmlProxyJumpers() const;

    int hostCount() const;

signals:
    void dataReset();
private slots:
    void onAboutToQuit();
    void onResetLater();

private:
    Q_INVOKABLE void init();
    QByteArray toStream();
    bool save(const HostInfo& hi);
    bool _renameGroup(const QString& nameNew, const QString& nameOld);
    bool _updateGroup(const QString& name, int order = 0);
    bool _removeGroup(const QString& name);
    void resetLater();
private:
    static QHash<QString, HostInfo> parse(const QByteArray& buf);
    static void importIdentityToSqlite(const QString& path, const QString& dbFile);
    static void importConfToSqlite(const QString& conf, const QString& dbFile);
    static QHash<QString, HostInfo> loadServerFromSqlite(const QString& dbFile);
    static QList<GroupInfo> loadGroupFromSqlite(const QString& dbFile);

private:
    QString m_dbFile;
    bool m_bInit;
    QHash<QString, HostInfo> m_hosts;
    QList<GroupInfo> m_groups;
    QPointer<QTimer> m_timer;
};
