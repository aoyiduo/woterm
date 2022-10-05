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

#ifndef QKXFTPSERVERPRIVATE_H
#define QKXFTPSERVERPRIVATE_H

#include <QObject>
#include <QPointer>
#include <QMap>

class QTcpServer;
class QIODevice;
class QKxFtpClient;
class QThread;
class QKxFtpServerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QKxFtpServerPrivate(const QString& envName);
    virtual ~QKxFtpServerPrivate();
private slots:
    void onNewConnection();
    void onDisconnected();
private:
    Q_INVOKABLE void init();
    Q_INVOKABLE void close();

private:
    QString m_envName;
    QPointer<QThread> m_worker;
    QPointer<QTcpServer> m_server;
    QMap<QPointer<QIODevice>, QPointer<QKxFtpClient> > m_clients;
    bool m_bClose;
};

#endif // QKXFTPSERVERPRIVATE_H
