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

#ifndef QKXFTPUPLOAD_H
#define QKXFTPUPLOAD_H

#include "qkxftp_share.h"

#include <QObject>
#include <QPointer>
#include <QCryptographicHash>

class QKxFtpRequest;
class QFile;
class KXFTP_EXPORT QKxFtpUpload : public QObject
{
    Q_OBJECT
public:
    explicit QKxFtpUpload(QKxFtpRequest *ftp, QObject *parent = nullptr);
    virtual ~QKxFtpUpload();
    void start(const QString& local, const QString& remote, bool fromZero);
    void stop();
signals:
    void error(int err, const QString& msg);
    void finished();
    void progress(int i, int pos, int fsize);
public slots:
    void onUploadInitResult(qint8 err, const QByteArray& msg, const QByteArray& md5, qint64 fsize, qint32 jobId);
    void onUploadRequest(qint8 err, const QByteArray& msg, qint32 jobId);
private:
    Q_INVOKABLE void verifyMd5(const QByteArray& md5, qint64 fsize, qint32 jobId);
private:
    qint32 m_jobId;
    QPointer<QKxFtpRequest> m_ftp;
    QPointer<QFile> m_file;
    QCryptographicHash m_crypt;
    QString m_local;
    QString m_remote;
    int m_percentLast;
    bool m_fromZero;
    qint64 m_tmStart;
    bool m_stop;
};

#endif // QKXFTPUPLOAD_H
