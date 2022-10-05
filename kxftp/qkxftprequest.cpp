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

#include "qkxftprequest.h"
#include "qkxftp_private.h"

#include <QVariantMap>

QKxFtpRequest::QKxFtpRequest(QObject *parent)
    : QObject(parent)
{
}

QKxFtpRequest::~QKxFtpRequest()
{

}

void QKxFtpRequest::mkdir(const QByteArray &path, const QByteArray &name)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_FILEMODEL_MKDIR_REQ) << path << name;
    writePacket(buf);
}

bool QKxFtpRequest::downloadInit(const QByteArray &fileName, const QByteArray& md5, qint64 fsize, qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_DOWNLOAD_INIT_REQ) << fileName << md5 << fsize << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::downloadStart(qint64 pos, qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_DOWNLOAD_DATA_START) << pos << jobId;
    writePacket(buf);
    return true;
}

void QKxFtpRequest::downloadRequest(qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_DOWNLOAD_DATA_NEXT) << jobId;
    writePacket(buf);
}

bool QKxFtpRequest::downloadAbort(qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_DOWNLOAD_DATA_ABORT) << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::downloadFinish(qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_DOWNLOAD_DATA_END) << jobId;
    writePacket(buf);
    return true;
}

void QKxFtpRequest::writePacket(const QByteArray &data)
{

}

void QKxFtpRequest::listFiles(const QByteArray &path)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_FILEMODEL_LIST_FILES_REQ) << path;
    writePacket(buf);
}

void QKxFtpRequest::entryInfoList(const QByteArray &path)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_FILEMODEL_ENTRY_INFO_LIST_REQ) << path;
    writePacket(buf);
}

bool QKxFtpRequest::uploadInit(const QByteArray &fileName, bool fromZero, qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_UPLOAD_INIT_REQ) << fileName << fromZero << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::uploadStart(qint64 pos, qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_UPLOAD_DATA_START) << pos << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::upload(const QByteArray &data, qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_UPLOAD_DATA_BLOCK) << data << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::uploadAbort(qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_UPLOAD_DATA_ABORT) << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::uploadFinish(qint32 jobId)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::ReadWrite);
    ds << qint8(FTP_UPLOAD_DATA_END) << jobId;
    writePacket(buf);
    return true;
}

bool QKxFtpRequest::handlePacket(const QByteArray& buf)
{
    append(buf);
    while(packetAvailable()) {
        QByteArray buf = nextPacket();
        QDataStream in(buf);
        if(!handleNextPacket(in)) {
            return false;
        }
    }
    return true;
}

bool QKxFtpRequest::handleNextPacket(QDataStream& in)
{
    qint8 type;
    in >> type;
    if(type == FTP_FILEMODEL_MKDIR_RESP) {
        qint8 err;
        QByteArray msg;
        in >> err >> msg;
        emit mkdirResult(err, msg);
    }else if(type == FTP_FILEMODEL_ENTRY_INFO_LIST_RESP) {
        qint8 err;
        QByteArray path, buf;
        QVariantList lsdm;
        in >> err >> path >> buf;
        if(err < 0) {
            emit entryInfoListResult(err, path, lsdm);
        }else{
            QDataStream ds(buf);
            qint32 cnt;
            ds >> cnt;
            for(int i = 0; i < cnt; i++) {
                qint8 type;
                qint64 fsize;
                QByteArray name;
                qint32 ct;
                ds >> type >> name >> fsize >> ct;
                QVariantMap dm;
                dm.insert("type", type);
                dm.insert("fsize", fsize);
                dm.insert("name", name);
                dm.insert("ct", ct);
                lsdm.append(dm);
            }
            emit entryInfoListResult(err, path, lsdm);
        }
    }else if(type == FTP_FILEMODEL_LIST_FILES_RESP) {
        qint8 err;
        QByteArray path;
        in >> err >> path;
        if(err < 0) {
            QByteArray reason;
            in >> reason;
            emit listResult(err, path, QByteArrayList());
        }else{
            QByteArrayList all;
            in >> all;
            emit listResult(err, path, all);
        }
        return true;
    }else if(type == FTP_DOWNLOAD_INIT_RESP){
        qint8 err = 0;
        QByteArray reason;
        qint64 fsize = 0;
        qint32 jobId = 0;
        in >> err;
        if(err < 0) {
            in >> reason;
        }else{
            in >> fsize;
        }
        in >> jobId;
        emit downloadInitResult(err, reason, fsize, jobId);
        return true;
    }else if(type == FTP_DOWNLOAD_DATA_BLOCK){
        qint8 err;
        QByteArray block;
        qint32 jobId;
        in >> err >> block >> jobId;
        emit downloadResult(err, block, jobId);
        return true;
    }else if(type == FTP_UPLOAD_INIT_RESP){
        qint8 err;
        in >> err;
        QByteArray md5;
        QByteArray reason;
        qint64 fsize = 0;
        qint32 jobId;
        if(err < 0) {
            in >> reason;
        }else{
            in >> md5 >> fsize;
        }
        in >> jobId;
        emit uploadInitResult(err, reason, md5, fsize, jobId);
        return true;
    }else if(type == FTP_UPLOAD_DATA_NEXT){
        qint8 err;
        qint32 jobId;
        in >> err;
        QByteArray reason;
        if(err < 0) {
            in >> reason;
        }
        in >> jobId;
        emit uploadRequest(err, reason, jobId);
        return true;
    }
    return true;
}
