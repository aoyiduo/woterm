/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QMOABOUTASSIST_H
#define QMOABOUTASSIST_H

#include <QObject>
#include <QPointer>

class QKxHttpClient;

class QMoAboutAssist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentVersion READ currentVersion)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestVersionArrived)
public:
    explicit QMoAboutAssist(QObject *parent = nullptr);
    virtual ~QMoAboutAssist();

    QString currentVersion() const;
    QString latestVersion() const;
    Q_INVOKABLE void upgradeVersion();
    Q_INVOKABLE void stopUpgrade();
    Q_INVOKABLE void checkLatestVersion();
signals:
    void latestVersionArrived();
    void upgradeMessageArrived(const QString& msg);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void installArrived(const QString& path);
    void finished();
private slots:
    void onVersionCheck(int code, const QByteArray &body);
    void onFileNameGet(int code, const QByteArray& body);
    void onDownloadFinished(int code, const QByteArray& body);
private:
    void findTargetFileName();
    void downloadFile(const QString& file);
    Q_INVOKABLE void init();

private:
    QString m_latestVersion;
    QString m_pathSave;
    QPointer<QKxHttpClient> m_http;
};

#endif // QMOABOUTASSIST_H
