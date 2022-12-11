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

#ifndef QWODBSFTPUPLOADSYNC_H
#define QWODBSFTPUPLOADSYNC_H

#include <QObject>
#include <QPointer>

#define DB_SFTP_UPLOAD_ENCRYPT_FILE         (1)
#define DB_SFTP_UPLOAD_CHECK_UPLOAD_PATH    (2)
#define DB_SFTP_UPLOAD_CHECK_VERSION        (3)
#define DB_SFTP_UPLOAD_WRITE_VERSION        (4)
#define DB_SFTP_UPLOAD_UPLOAD_FILE          (5)
#define DB_SFTP_UPLOAD_FATAL                (99)

class QWoSshFtp;
class QWoDBSftpUploadSync : public QObject
{
    Q_OBJECT
public:
    explicit QWoDBSftpUploadSync(QObject *parent = nullptr);
    virtual ~QWoDBSftpUploadSync();
    void upload(const QString& type, const QString& key);
signals:
    void infoArrived(int action, int err, const QString& errDesc);
private slots:
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onFinishArrived(int code);
    void onInputArrived(const QString &title, const QString &prompt, bool visible);
private:
    void reconnect();
    void release();
    void removeTempFile();
    void runAction(int action, const QString& tip);
    void finishAction(int err, const QString& errDesc);
private:
    QPointer<QWoSshFtp> m_sftp;
    QString m_pathUpload, m_pathTemp, m_nameCrypt;
    int m_action;
};

#endif // QWODBSFTPUPLOADSYNC_H
