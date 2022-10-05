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

#ifndef QKXHTTPCLIENT_H
#define QKXHTTPCLIENT_H

#include "qkxutil_share.h"

#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class KXUTIL_EXPORT QKxHttpClient : public QObject
{
    Q_OBJECT
public:
    explicit QKxHttpClient(QObject *parent = nullptr);
    virtual ~QKxHttpClient();
    bool get(const QString& url);
    bool post(const QString& url, const QByteArray& data);
signals:
    void result(int code, const QByteArray& body);
    void finished();
private slots:
    void onReadyRead();
    void onError();
    void onSslError();
    void onFinished();
private:
    QPointer<QNetworkAccessManager> m_manager;
    QPointer<QNetworkReply> m_reply;
    QByteArray m_data;
};

#endif // QKXHTTPCLIENT_H
