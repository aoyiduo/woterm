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

#ifndef QKXLOCALPEER_H
#define QKXLOCALPEER_H

#include "qkxutil_share.h"

#include <QObject>


class QKxLocalPeerPrivate;
class KXUTIL_EXPORT QKxLocalPeer : public QObject
{
    Q_OBJECT
public:
    explicit QKxLocalPeer(const QString& id, bool withSid, QObject *parent = nullptr);
    explicit QKxLocalPeer(const QString& id, QObject *parent = nullptr);
    ~QKxLocalPeer();
    bool isClient();
    bool sendMessage(const QString& msg, int timeout = 500);
signals:
    void messageReceived(const QString &message);
private slots:
    void onNewConnection();
    void onReadReady();
private:
    QKxLocalPeerPrivate *m_prv;
};

#endif // QKXLOCALPEER_H
