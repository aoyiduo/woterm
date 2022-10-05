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

#include "qkxftpupload.h"
#include "qkxftprequest.h"

#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

#define MAX_BLOCK_SIZE      (256 * 1024)

QKxFtpUpload::QKxFtpUpload(QKxFtpRequest *ftp, QObject *parent)
    : QObject(parent)
    , m_ftp(ftp)
    , m_crypt(QCryptographicHash::Md5)
    , m_jobId(0)
{
    QObject::connect(ftp, SIGNAL(uploadInitResult(qint8,QByteArray,QByteArray,qint64,qint32)), this, SLOT(onUploadInitResult(qint8,QByteArray,QByteArray,qint64,qint32)));
    QObject::connect(ftp, SIGNAL(uploadRequest(qint8,QByteArray,qint32)), this, SLOT(onUploadRequest(qint8,QByteArray,qint32)));
}

QKxFtpUpload::~QKxFtpUpload()
{

}

void QKxFtpUpload::start(const QString &local, const QString &remote, bool fromZero)
{
    m_local = local;
    m_remote = remote;
    m_stop = false;
    m_percentLast = 0;
    m_fromZero = fromZero;
    if(m_file) {
        m_file->close();
        m_file->deleteLater();
    }
    m_crypt.reset();
    m_jobId++;
    m_tmStart = QDateTime::currentMSecsSinceEpoch();
    m_ftp->uploadInit(remote.toUtf8(), fromZero, m_jobId);
}

void QKxFtpUpload::stop()
{
    m_stop = true;
    if(m_file){
        m_file->close();
        m_file->deleteLater();
    }
    m_file = nullptr;
}

void QKxFtpUpload::onUploadInitResult(qint8 err, const QByteArray& msg, const QByteArray& md5, qint64 fsize, qint32 jobId)
{
    if(m_jobId != jobId) {
        return;
    }
    qDebug() << err << fsize << msg;
    if(err < 0) {
        emit error(err + 50, msg);
        qDebug() << "onUploadInitResult" << msg;
    }else {
        if(m_file != nullptr) {
            m_file->close();
            m_file->deleteLater();
        }
        m_file = new QFile(m_local, this);
        if(!m_file->open(QIODevice::ReadOnly)) {
            emit error(-1, tr("Failed to open file:")+m_local);
            m_ftp->uploadAbort(jobId);
            return;
        }
        if(fsize == 0 || m_fromZero){
            emit progress(0, 0, m_file->size());
            m_ftp->uploadStart(0, jobId);
        }else{
            verifyMd5(md5, fsize, jobId);
        }
    }
}

void QKxFtpUpload::onUploadRequest(qint8 err, const QByteArray &msg, qint32 jobId)
{
    if(m_file == nullptr || m_jobId != jobId) {
        return;
    }
    if(m_stop) {
        m_ftp->downloadAbort(jobId);
        return;
    }
    if(err < 0) {
        m_ftp->downloadAbort(jobId);
        emit error(err-10, msg);
        return;
    }
    QByteArray data = m_file->read(MAX_BLOCK_SIZE);
    qint64 fsize = m_file->size();
    int percent = (fsize > 0) ? (m_file->pos() * 100 / m_file->size()) : 0;
    //qDebug() << "onUploadResult" << percent << m_file->pos() << data.length();
    if(percent != m_percentLast) {
        m_percentLast = percent;
        emit progress(percent, m_file->pos(), fsize);
    }
    if(!data.isEmpty()) {
        m_ftp->upload(data, jobId);
    }else {
        m_ftp->uploadFinish(jobId);
        qint64 fsize = m_file->size();
        qint64 elapse = QDateTime::currentMSecsSinceEpoch() - m_tmStart;
        int bytePer = elapse > 0 ? fsize * 1000 / elapse : 0;
        qDebug() << "finish task used" << elapse << fsize << bytePer;
        emit finished();
    }
}

void QKxFtpUpload::verifyMd5(const QByteArray& md5, qint64 fsize, qint32 jobId)
{
    if(m_file == nullptr || m_jobId != jobId) {
        return;
    }
    qint64 pos = m_file->pos();
    int size = MAX_BLOCK_SIZE;
    if(pos + size > fsize) {
        size = fsize - pos;
    }
    QByteArray buf = m_file->read(size);
    m_crypt.addData(buf);
    if(m_file->pos() < fsize) {
        QMetaObject::invokeMethod(this, "verifyMd5", Qt::QueuedConnection,
                                  Q_ARG(QByteArray, md5), Q_ARG(qint64,fsize), Q_ARG(qint32, jobId));
    }else{
        QByteArray tmp = m_crypt.result();
        qint64 pos = 0;
        if(tmp == md5 && fsize > 0) {
            pos = fsize;
        }else{
            m_file->seek(0);
        }
        m_ftp->uploadStart(pos, jobId);
    }
}
