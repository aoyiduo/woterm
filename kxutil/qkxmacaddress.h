/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QKXMACADDRESS_H
#define QKXMACADDRESS_H

#include "qkxutil_share.h"

#include <QPointer>
#include <QObject>
#include <QMap>

class QProcess;
class QTimer;
class QTcpSocket;
class KXUTIL_EXPORT QKxMACAddress : public QObject
{
    Q_OBJECT
public:
    explicit QKxMACAddress(QObject *parent = nullptr);
    void start(const QString& host, quint16 port);
    void restart();
    static QMap<QString, QString> getCandidate();
signals:
    void macArrived(const QString& mac);
    void errorArrived(const QString& err);
public slots:
    void onError();
    void onConnected();
    void onDisconnected();
    void onNetstatResult();
    void onTimeout();
private:
    Q_INVOKABLE void init();
private:
    void clean();
    void sendError(const QString& err);
    void sendResult(const QString& mac);
private:
    QString m_host;
    quint16 m_port;
    QMap<QString, QString> m_ipToMac;
    QString m_macUsed;
    bool m_hasSendResult;
    QPointer<QProcess> m_proc;
    QPointer<QTcpSocket> m_tcp;
    QPointer<QTimer> m_timer;
};

#endif // QKXMACADDRESS_H
