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

#ifndef QKXFTPDOWNLOAD_H
#define QKXFTPDOWNLOAD_H

#include "qkxftp_share.h"

#include <QObject>
#include <QPointer>
#include <QCryptographicHash>

class QKxFtpRequest;
class QFile;
class KXFTP_EXPORT QKxFtpDownload : public QObject
{
    Q_OBJECT
public:
    explicit QKxFtpDownload(QKxFtpRequest *ftp, QObject *parent = nullptr);
    virtual ~QKxFtpDownload();
    void start(const QString& local, const QString& remote, bool fromZero);
    void stop();
signals:
    void error(int err, const QString& msg);
    void finished();
    void progress(int i, int pos, int fsize);
public slots:
    void onDownloadInitResult(qint8 err, const QByteArray& msg, qint64 fsize, qint32 jobId);
    void onDownloadResult(qint8 err, const QByteArray& data, qint32 jobId);
private:
    Q_INVOKABLE void verifyMd5(qint32 jobId);
private:
    qint32 m_jobId;
    QPointer<QKxFtpRequest> m_ftp;
    QPointer<QFile> m_file;
    QCryptographicHash m_crypt;
    QString m_local;
    QString m_remote;
    qint64 m_fileSize;
    int m_percentLast;
    bool m_fromZero;
    bool m_stop;

    qint64 m_tmStart;
};

#endif // QKXFTPDOWNLOAD_H
