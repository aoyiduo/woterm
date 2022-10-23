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
#include <QHash>

class QBuffer;

class QWoSshConf : public QObject
{
    Q_OBJECT
public:
    static QWoSshConf* instance();
    static bool databaseValid(const QString& dbFile);

    bool restore(const QString& dbFile);
    bool backup(const QString& dbFile);
    bool refresh();
    bool exportToFile(const QString& path);
    bool remove(const QString& name);
    bool modify(const HostInfo& hi);
    bool append(const HostInfo& hi);
    bool modifyOrAppend(const HostInfo& hi);
    HostInfo find(const QString&name) const;
    bool find(const QString &name, HostInfo* pinfo) const;


    void resetAllProperty(const QString& v);
    bool exists(const QString& name) const;
    void updatePassword(const QString& name, const QString& password);

    QList<HostInfo> hostList(EHostType type = All) const;
    QStringList hostNameList(EHostType type = All) const;
    QList<HostInfo> proxyJumpers(const QString& name, int max=2) const;

private slots:
    void onAboutToQuit();

protected:
    explicit QWoSshConf(const QString& dbFile, QObject *parent = nullptr);
private:
    Q_INVOKABLE void init();
    QByteArray toStream();
    bool save(const HostInfo& hi);
private:
    static bool initialize(const QString& dbFile);      
    static QHash<QString, HostInfo> parse(const QByteArray& buf);
    static void importIdentityToSqlite(const QString& path, const QString& dbFile);
    static void importConfToSqlite(const QString& conf, const QString& dbFile);
    static QHash<QString, HostInfo> loadFromSqlite(const QString& dbFile);

private:
    QString m_dbFile;
    bool m_bInit;
    QHash<QString, HostInfo> m_hosts;
};
