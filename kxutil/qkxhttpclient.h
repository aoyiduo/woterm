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

#ifndef QKXHTTPCLIENT_H
#define QKXHTTPCLIENT_H

#include "qkxutil_share.h"

#include <QObject>
#include <QPointer>

class QFile;
class QNetworkAccessManager;
class QNetworkReply;
class KXUTIL_EXPORT QKxHttpClient : public QObject
{
    Q_OBJECT
public:
    explicit QKxHttpClient(QObject *parent = nullptr);
    virtual ~QKxHttpClient();
    bool get(const QString& url);
    bool post(const QString& url, const QByteArray& data, const QString& contentType = "application/text");
    bool fileGet(const QString& url, const QString& fileSave);
    bool filePost(const QString& url, const QByteArray& data, const QString& fileSave);
    QString fileSavePath() const;
    QString lastErrorString() const;
signals:
    void result(int code, const QByteArray& body);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void finished();
private slots:
    void onReadyRead();
    void onError();
    void onSslError();
    void onFinished();
private:
    QPointer<QNetworkAccessManager> m_manager;
    QPointer<QNetworkReply> m_reply;
    QString m_lastError;
    QByteArray m_data;
    QString m_fileSave;
    QPointer<QFile> m_file;
};

#endif // QKXHTTPCLIENT_H
