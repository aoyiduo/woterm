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
public:
    explicit QWoSshConf(const QString& conf, QObject *parent = nullptr);

    static QWoSshConf* instance();

    bool save();
    bool refresh();
    bool exportToFile(const QString& path);
    void remove(const QString& name);
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
private:
    QHash<QString, HostInfo> parse(const QByteArray& buf);
    QByteArray toStream();
private:
    QString m_conf;
    QHash<QString, HostInfo> m_hosts;
};
