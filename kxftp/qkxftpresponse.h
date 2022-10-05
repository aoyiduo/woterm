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

#ifndef QKXFTPRESPONSE_H
#define QKXFTPRESPONSE_H

#include "qkxftp_share.h"
#include "qkxlengthbodypacket.h"

#include <QObject>
#include <QPointer>
#include <QDataStream>
#include <QCryptographicHash>


class QIODevice;
class QLocalSocket;
class QFile;

class KXFTP_EXPORT QKxFtpResponse : public QObject,
        public QKxLengthBodyPacket
{
    Q_OBJECT
public:
    explicit QKxFtpResponse(QObject *parent = nullptr);
    ~QKxFtpResponse();

    void handlePacket(const QByteArray& buf);
signals:
    void result(const QByteArray& buf);
protected:
    virtual void writeResult(const QByteArray& buf);
private:
    Q_INVOKABLE void downloadVerify(const QByteArray& md5, qint64 fsize, qint32 jobId);
    Q_INVOKABLE void uploadVerify(qint32 jobId);
    bool tryTohandlePacket();
    bool handleNextPacket(QDataStream& in, QDataStream& out);
    void sendFileBlock(qint64 pos, qint32 jobId, QDataStream& out);
    bool listFiles(const QByteArray &path, QByteArrayList& all);
    bool listFiles(const QString &path, QStringList &all, int maxCount);
    QByteArray entryInfoList(QByteArray& path);
private:
    // transfer
    QCryptographicHash m_crypt;
    QPointer<QFile> m_file;
    qint32 m_recvLeft;
    QByteArray m_recvBuffer;
};

#endif // QKXFTPRESPONSE_H
