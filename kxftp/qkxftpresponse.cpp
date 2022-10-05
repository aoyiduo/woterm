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

#include "qkxftpresponse.h"
#include "qkxftp_private.h"

#include <QLocalSocket>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>

#define MAX_BLOCK_SIZE  (256*1024)

QKxFtpResponse::QKxFtpResponse(QObject *parent)
    : QObject(parent)
    , QKxLengthBodyPacket(true)
    , m_crypt(QCryptographicHash::Md5)
    , m_recvLeft(0)
{
    m_recvBuffer.reserve(1024);
}

QKxFtpResponse::~QKxFtpResponse()
{
}

void QKxFtpResponse::handlePacket(const QByteArray &buf)
{
    append(buf);
    tryTohandlePacket();
}

void QKxFtpResponse::writeResult(const QByteArray &buf)
{
    emit result(buf);
}

void QKxFtpResponse::downloadVerify(const QByteArray &md5, qint64 fsize, qint32 jobId)
{
    qint64 pos = m_file->pos();
    int size = MAX_BLOCK_SIZE;
    if(pos + size > fsize) {
        size = fsize - pos;
    }
    QByteArray buf = m_file->read(size);
    m_crypt.addData(buf);
    if(m_file->pos() < fsize) {
        QMetaObject::invokeMethod(this, "downloadVerify", Qt::QueuedConnection,
                                  Q_ARG(QByteArray, md5), Q_ARG(qint64, fsize), Q_ARG(qint32, jobId));
    }else{
        QByteArray tmp = m_crypt.result();
        QByteArray buf;
        QDataStream out(&buf, QIODevice::ReadWrite);
        qint8 way = 0;
        if(tmp == md5 && fsize > 0) {
            way = 1; // continue.
        }else {
            m_file->seek(0);
        }
        out << qint8(FTP_DOWNLOAD_INIT_RESP) << qint8(way) << m_file->size() << jobId;

        writeResult(buf);
    }
}

void QKxFtpResponse::uploadVerify(qint32 jobId)
{
    QByteArray buf = m_file->read(MAX_BLOCK_SIZE);
    m_crypt.addData(buf);
    if(!m_file->atEnd()) {
        QMetaObject::invokeMethod(this, "uploadVerify", Qt::QueuedConnection, Q_ARG(qint32, jobId));
    }else{
        QByteArray md5 = m_crypt.result();
        QByteArray buf;
        QDataStream out(&buf, QIODevice::ReadWrite);
        out << qint8(FTP_UPLOAD_INIT_RESP) << qint8(0) << md5 << m_file->size() << jobId;
        writeResult(buf);
    }
}

bool QKxFtpResponse::tryTohandlePacket()
{
    while(packetAvailable()) {
        QByteArray raw = nextPacket();
        if(raw.isEmpty()) {
            return true;
        }
        QDataStream in(raw);
        QByteArray buf;
        QDataStream out(&buf, QIODevice::ReadWrite);
        if(!handleNextPacket(in, out)) {
            return false;
        }
        if(!buf.isEmpty()) {
            writeResult(buf);
        }
    }
    return true;
}

bool QKxFtpResponse::handleNextPacket(QDataStream &in, QDataStream &out)
{
    qint8 type;
    in >> type;
    if(type== FTP_FILEMODEL_MKDIR_REQ) {
        QByteArray path, name;
        in >> path >> name;
        QString tmp = QDir::cleanPath(path + "/");
        QDir d(tmp);
        if(!d.exists()) {
            out << qint8(FTP_FILEMODEL_MKDIR_RESP) << qint8(-1) << QByteArray("the directory is not exist");
        }else if(!d.mkdir(name)) {
            out << qint8(FTP_FILEMODEL_MKDIR_RESP) << qint8(-2) << QByteArray("failed to mkdir for no permission.");
        }
        out << qint8(FTP_FILEMODEL_MKDIR_RESP) << qint8(0) << QByteArray("ok.");
        return true;
    }else if(type == FTP_FILEMODEL_LIST_FILES_REQ) {
        QByteArray path;
        in >> path;
        QByteArrayList all;
        if(!listFiles(path, all)) {
            out << qint8(FTP_FILEMODEL_LIST_FILES_RESP) << qint8(-1) << path << QByteArray("too many file to load");
        }else{
            out << qint8(FTP_FILEMODEL_LIST_FILES_RESP) << qint8(0) << path << all;
        }
        return true;
    }else if(type == FTP_FILEMODEL_ENTRY_INFO_LIST_REQ) {
        QByteArray path;
        in >> path;
#ifndef Q_OS_WIN
        if(path == "~") {
            path = QDir::homePath().toUtf8();
        }else{
            QString root = QDir::homePath();
            if(!path.startsWith(root.toUtf8())) {
                path = root.toUtf8();
            }
        }
#endif
        QByteArray result = entryInfoList(path);
        if(result.isEmpty()) {
            out << qint8(FTP_FILEMODEL_ENTRY_INFO_LIST_RESP) << qint8(-1) << path << QByteArray("not exist");
        }else {
            out << qint8(FTP_FILEMODEL_ENTRY_INFO_LIST_RESP) << qint8(0) << path << result;
        }
    }else if(type == FTP_DOWNLOAD_INIT_REQ){
        QByteArray fileName, md5;
        qint32 jobId;
        qint64 fsize;
        in >> fileName >> md5 >> fsize >> jobId;
        if(m_file != nullptr) {
            m_file->close();
            m_file->deleteLater();
        }
        m_file = new QFile(QString::fromUtf8(fileName), this);
        if(!m_file->open(QIODevice::ReadOnly)) {
            QString reason = m_file->errorString();
            out << qint8(FTP_DOWNLOAD_INIT_RESP) << qint8(-1) << reason.toUtf8() << jobId;
        }else{
            downloadVerify(md5, fsize, jobId);
        }
        return true;
    }else if(type == FTP_DOWNLOAD_DATA_START){
        qint64 pos;
        qint32 jobId;
        in >> pos >> jobId;
        sendFileBlock(pos, jobId, out);
        return true;
    }else if(type == FTP_DOWNLOAD_DATA_NEXT){
        qint32 jobId;
        in >> jobId;
        qint64 pos = m_file->pos();
        sendFileBlock(pos, jobId, out);
        return true;
    }else if(type == FTP_DOWNLOAD_DATA_END || type == FTP_DOWNLOAD_DATA_ABORT){
        qint32 jobId;
        in >> jobId;
        if(m_file){
            m_file->close();
            m_file->deleteLater();
        }
        return true;
    }else if(type == FTP_UPLOAD_INIT_REQ) {
        QByteArray fileName;
        qint32 jobId;
        bool fromZero;
        in >> fileName >> fromZero >> jobId;
        if(m_file != nullptr) {
            m_file->close();
            m_file->deleteLater();
        }
        QString file = QString::fromUtf8(fileName);
        if(fromZero) {
            QFile::remove(file);
        }
        m_file = new QFile(file, this);
        if(!m_file->open(QIODevice::ReadWrite)) {
            QString reason = m_file->errorString();
            out << qint8(FTP_UPLOAD_INIT_RESP) << qint8(-2) << reason.toUtf8() << jobId;
        }else if(m_file->size() == 0 || fromZero){
            out << qint8(FTP_UPLOAD_INIT_RESP) << qint8(0) << QByteArray() << qint64(0) << jobId;
        }else{
            uploadVerify(jobId);
        }
        return true;
    }else if(type == FTP_UPLOAD_DATA_START) {
        qint64 pos;
        qint32 jobId;
        in >> pos >> jobId;
        if(m_file) {
            if(!m_file->seek(pos)) {
                out << qint8(FTP_UPLOAD_DATA_NEXT) << qint8(-1) << QByteArray("failed to do seek operation.") << jobId;
            }else{
                out << qint8(FTP_UPLOAD_DATA_NEXT) << qint8(0) << jobId;
            }
        }
        return true;
    }else if(type == FTP_UPLOAD_DATA_BLOCK) {
        QByteArray block;
        qint32 jobId;
        in >> block >> jobId;
        if(m_file) {
            if(m_file->write(block) != block.length()) {
                out << qint8(FTP_UPLOAD_DATA_NEXT) << qint8(-2) << QByteArray("not enough space to write.") << jobId;
            }else{
                out << qint8(FTP_UPLOAD_DATA_NEXT) << qint8(0) << jobId;
            }
        }
        return true;
    }else if(type == FTP_UPLOAD_DATA_END || type == FTP_UPLOAD_DATA_ABORT) {
        qint32 jobId;
        in >> jobId;
        if(m_file){
            m_file->close();
            m_file->deleteLater();
        }
        return true;
    }
    return true;
}

void QKxFtpResponse::sendFileBlock(qint64 pos, qint32 jobId, QDataStream &out)
{
    if(!m_file->isOpen()) {
        QByteArray reason = QByteArray("bad request")+QByteArray::number(pos);
        out << qint8(FTP_DOWNLOAD_DATA_BLOCK) << qint8(-1) << reason << jobId;
        return;
    }
    if(!m_file->seek(pos)) {
        QByteArray reason = QByteArray("failed to seek pos")+QByteArray::number(pos);
        out << qint8(FTP_DOWNLOAD_DATA_BLOCK) << qint8(-2) << reason << jobId;
        return;
    }
    QByteArray block = m_file->read(MAX_BLOCK_SIZE);
    out << qint8(FTP_DOWNLOAD_DATA_BLOCK) << qint8(0) << block << jobId;
}

bool QKxFtpResponse::listFiles(const QByteArray &path, QByteArrayList &all)
{
    QStringList files;
    bool ok = listFiles(path, files, 500);
    all.clear();
    for(int i = 0; i < files.length(); i++) {
        QString tmp = files.at(i);
        tmp = tmp.remove(path);
        all.append(tmp.toUtf8());
    }
    return ok;
}

bool QKxFtpResponse::listFiles(const QString &path, QStringList &all, int maxCount)
{
    QFileInfo fi(path);
    if(fi.isDir()) {
        QDir d(fi.absoluteFilePath());
        QFileInfoList fil = d.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
        for(int i = 0; i < fil.length(); i++) {
            QFileInfo fi = fil.at(i);
            if(!listFiles(fi.absoluteFilePath(), all, maxCount)) {
                return false;
            }
        }
    }else{
        all.append(fi.absoluteFilePath());
    }
    return all.length() < maxCount;
}


#define FILE_FLAG_FILE          (1)
#define FILE_FLAG_DIR           (1<<1)
#define FILE_FLAG_ROOT          (1<<2)
QByteArray QKxFtpResponse::entryInfoList(QByteArray &path)
{
    QFileInfoList fis;
    if(path.isEmpty()) {
#ifdef Q_OS_WIN
        fis = QDir::drives();
#else
        QDir d("/");
        fis = d.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
#endif
    }else{
        path = QDir::cleanPath(path + "/").toUtf8();
        QDir d(path);
        if(!d.exists()) {
            return QByteArray();
        }
        fis = d.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    }
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << qint32(fis.length());
    for(int i = 0; i < fis.length(); i++) {
        QFileInfo fi = fis.at(i);
        qint8 type = 0;
        if(fi.isFile()) {
            type |= FILE_FLAG_FILE;
        }
        if(fi.isDir()) {
            type |= FILE_FLAG_DIR;
        }
        if(fi.isRoot()) {
            type |= FILE_FLAG_ROOT;
        }
        QByteArray fileName = fi.fileName().toUtf8();
        if(fileName.isEmpty()) {
            fileName = fi.path().toUtf8();
        }
        qint64 fsize = fi.size();
        QDateTime dt = fi.created();
        qint32 ct = dt.toSecsSinceEpoch();
        ds << type << fileName << fi.size() << ct;
    }
    return buf;
}
