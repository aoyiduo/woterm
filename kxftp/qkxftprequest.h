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

#ifndef QKXFTPREQUESTIMPL_H
#define QKXFTPREQUESTIMPL_H

#include "qkxftp_share.h"
#include "qkxlengthbodypacket.h"

#include <QPointer>

class QKxLengthBodyPacket;
class KXFTP_EXPORT QKxFtpRequest : public QObject, protected QKxLengthBodyPacket
{
    Q_OBJECT
public:
    explicit QKxFtpRequest(QObject *parent = nullptr);
    ~QKxFtpRequest();

    //file system.
    void mkdir(const QByteArray& path, const QByteArray& name);
    void listFiles(const QByteArray&  path);
    void entryInfoList(const QByteArray& path);
    //upload
    bool uploadInit(const QByteArray& fileName, bool fromZero, qint32 jobId);
    bool uploadStart(qint64 pos, qint32 jobId);
    bool upload(const QByteArray& data, qint32 jobId);
    bool uploadAbort(qint32 jobId);
    bool uploadFinish(qint32 jobId);
    //down
    bool downloadInit(const QByteArray& fileName, const QByteArray& md5, qint64 fsize, qint32 jobId);
    bool downloadStart(qint64 pos, qint32 jobId);
    void downloadRequest(qint32 jobId);
    bool downloadAbort(qint32 jobId);
    bool downloadFinish(qint32 jobId);
signals:
    void error(int err, const QString &msg);
    void error(int err, const QByteArray& msg);
    void disconnectedArrived();
    void connectedArrived();

    void mkdirResult(qint8 err, const QByteArray& msg);
    void listResult(qint8 err, const QByteArray & path, const QByteArrayList & files);
    void entryInfoListResult(qint8 err, const QByteArray& path, const QVariantList& files);

    // upload
    void uploadInitResult(qint8 err, const QByteArray& msg, const QByteArray& md5, qint64 fsize, qint32 jobId);
    void uploadRequest(qint8 err, const QByteArray& msg, qint32 jobId);
    // download
    void downloadInitResult(qint8 err, const QByteArray& msg, qint64 fsize, qint32 jobId);
    void downloadResult(qint8 err, const QByteArray& data, qint32 jobId);
protected:
    virtual void writePacket(const QByteArray& data);
protected:
    bool handlePacket(const QByteArray& data);
    bool handleNextPacket(QDataStream& in);
};

#endif // QKXFTPREQUESTIMPL_H
