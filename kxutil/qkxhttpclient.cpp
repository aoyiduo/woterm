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

#include "qkxhttpclient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkRequest>

QKxHttpClient::QKxHttpClient(QObject *parent)
    : QObject(parent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    manager->setProxy(QNetworkProxy::NoProxy);
    m_manager = manager;
}

QKxHttpClient::~QKxHttpClient()
{
    qDebug() << "deleteLater";
}

bool QKxHttpClient::get(const QString &url)
{
    if(m_reply != nullptr && !m_reply->isFinished()) {
        return false;
    }
    if(m_reply != nullptr) {
        m_reply->deleteLater();
    }
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(url)));
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError()));
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    m_reply = reply;
    return true;
}

bool QKxHttpClient::post(const QString &url, const QByteArray &data)
{
    if(m_reply != nullptr || !m_reply->isFinished()) {
        return false;
    }
    if(m_reply != nullptr) {
        m_reply->deleteLater();
    }
    QNetworkReply *reply = m_manager->post(QNetworkRequest(QUrl(url)), data);
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError()));
    m_reply = reply;
    return true;
}

void QKxHttpClient::onReadyRead()
{
    m_data.append(m_reply->readAll());
}

void QKxHttpClient::onError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    qInfo() << reply->url() << reply->errorString();
}

void QKxHttpClient::onSslError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    qInfo() << reply->url() << reply->errorString();
}

void QKxHttpClient::onFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QVariant code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qInfo() << reply->url() << code;
    emit result(code.toInt(), m_data);
    reply->deleteLater();
    emit finished();
}
