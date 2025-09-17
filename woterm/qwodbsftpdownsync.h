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

#ifndef QWODBSFTPDOWNSYNC_H
#define QWODBSFTPDOWNSYNC_H

#include <QObject>
#include <QPointer>

#define DB_SFTP_DOWNLOAD_CHECK_VERSION        (1)
#define DB_SFTP_DOWNLOAD_DOWNLOAD_FILE        (2)

class QWoSshFtp;
class QWoDBSftpDownSync : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath)
public:
    explicit QWoDBSftpDownSync(QObject *parent = nullptr);
    ~QWoDBSftpDownSync();
    QString downloadPath() const;
    void setDownloadPath(const QString& path);

    Q_INVOKABLE void listAll();
    Q_INVOKABLE void fetchLatest();
    Q_INVOKABLE void fetch(const QString& fileName);
    Q_INVOKABLE QVariant qmlDecryptFile(const QString &fileNameSrc, const QString &fileNameDst, const QString& cryptType, const QString& cryptKey);
    Q_INVOKABLE QVariant qmlAbsolutePath(const QString& fileName);
signals:
    void infoArrived(int action, int err, const QString& errDesc);
    void listArrived(const QStringList& fileNames);
    void fetchFinished(int err, const QString& fileName);
private slots:
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onFinishArrived(int code);
    void onInputArrived(const QString &title, const QString &prompt, bool visible);
    void onDirOpen(const QString& path, const QVariantList& data, const QVariantMap& user);
private:
    void release();
    void reconnect();
    void download(const QString& fileName);
    void removeTempFile();
    void runAction(int action, const QString& tip);
    void finishAction(int err, const QString& errDesc);
    bool decryptFile(const QString &fileNameSrc, const QString &fileNameDst, const QString& cryptType, const QString& cryptKey, QString& errMsg);
    bool decrypt(const QByteArray& in, const QByteArray &type, const QByteArray &key, QByteArray &out);
private:
    QPointer<QWoSshFtp> m_sftp;
    QString m_pathUpload, m_pathTemp, m_nameCrypt;
    int m_action;
};

#endif // QWODBSFTPDOWNSYNC_H
