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

#include "qkxhttpclient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QFile>

QKxHttpClient::QKxHttpClient(QObject *parent)
    : QObject(parent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    manager->setProxy(QNetworkProxy::NoProxy);
    m_manager = manager;
}

QKxHttpClient::~QKxHttpClient()
{
    if(m_manager) {
        delete m_manager;
    }
    qDebug() << "deleteLater";
}

bool QKxHttpClient::get(const QString &_url)
{
    QString url = _url.simplified();

    //QString ssl = QSslSocket::sslLibraryBuildVersionString();

    if(m_reply != nullptr && !m_reply->isFinished()) {
        return false;
    }
    if(m_file) {
        m_file->deleteLater();
    }
    m_fileSave.clear();
    if(m_reply != nullptr) {
        m_reply->deleteLater();
    }
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(url)));
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError()));
    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    m_reply = reply;
    m_lastError.clear();
    return true;
}

bool QKxHttpClient::post(const QString &url, const QByteArray &data, const QString& contentType)
{
    if(m_reply != nullptr && !m_reply->isFinished()) {
        return false;
    }
    if(m_file) {
        m_file->deleteLater();
    }
    m_fileSave.clear();
    if(m_reply != nullptr) {
        m_reply->deleteLater();
    }

    QNetworkRequest req = QNetworkRequest(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    QNetworkReply *reply = m_manager->post(req, data);
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslError()));
    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    m_reply = reply;
    m_lastError.clear();
    return true;
}

bool QKxHttpClient::fileGet(const QString &url, const QString &fileSave)
{
    QFile::remove(fileSave);
    bool ok = get(url);
    m_fileSave = fileSave;
    return ok;
}

bool QKxHttpClient::filePost(const QString &url, const QByteArray &data, const QString &fileSave)
{
    QFile::remove(fileSave);
    bool ok = post(url, data);
    m_fileSave = fileSave;
    return ok;
}

QString QKxHttpClient::fileSavePath() const
{
    return m_fileSave;
}

QString QKxHttpClient::lastErrorString() const
{
    return m_lastError;
}

void QKxHttpClient::onReadyRead()
{
    QByteArray all = m_reply->readAll();
    if(m_fileSave.isEmpty()) {
        m_data.append(all);
    }else if(m_file == nullptr){
        m_file = new QFile(m_fileSave);
        if(m_file->open(QFile::WriteOnly)) {
            m_file->write(all);
        }
    }else if(m_file->isOpen()){
        m_file->write(all);
    }
}

void QKxHttpClient::onError()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    m_lastError = reply->errorString();
    qInfo() << reply->url() << m_lastError;

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
    if(!code.isValid()) {
        m_lastError = reply->errorString();
    }
    if(m_file) {
        m_file->close();
        m_file->deleteLater();
    }
    emit result(code.toInt(), m_data);
    reply->deleteLater();
    emit finished();
}
