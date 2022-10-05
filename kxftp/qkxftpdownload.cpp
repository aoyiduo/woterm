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

#include "qkxftpdownload.h"
#include "qkxftprequest.h"

#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

#define MAX_BLOCK_SIZE      (256 * 1024)

QKxFtpDownload::QKxFtpDownload(QKxFtpRequest *ftp, QObject *parent)
    : QObject(parent)
    , m_ftp(ftp)
    , m_crypt(QCryptographicHash::Md5)
    , m_jobId(0)
{
    QObject::connect(ftp, SIGNAL(downloadInitResult(qint8,QByteArray,qint64,qint32)), this, SLOT(onDownloadInitResult(qint8,QByteArray,qint64,qint32)));
    QObject::connect(ftp, SIGNAL(downloadResult(qint8,QByteArray,qint32)), this, SLOT(onDownloadResult(qint8,QByteArray,qint32)));
}

QKxFtpDownload::~QKxFtpDownload()
{

}

void QKxFtpDownload::start(const QString &local, const QString &remote, bool fromZero)
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
    if(!QFileInfo::exists(local)) {
        int ipos = local.lastIndexOf('/');
        QString path = local.mid(0, ipos);
        QDir d(path);
        if(!d.exists()) {
            d.mkpath(path);
        }
    }
    m_file = new QFile(local);
    if(!m_file->open(QIODevice::ReadWrite)) {
        emit error(-1, tr("Failed to open file"));
        return;
    }
    m_crypt.reset();
    m_jobId++;
    if(m_file->size() == 0 || m_fromZero) {
        m_ftp->downloadInit(m_remote.toUtf8(), QByteArray(), 0, m_jobId);
    }else{
        verifyMd5(m_jobId);
    }
}

void QKxFtpDownload::stop()
{
    m_stop = true;
    if(m_file){
        m_file->close();
        m_file->deleteLater();
    }
    m_file = nullptr;
}

void QKxFtpDownload::onDownloadInitResult(qint8 err, const QByteArray &msg, qint64 fsize, qint32 jobId)
{
    if(m_file == nullptr || m_jobId != jobId) {
        return;
    }
    m_tmStart = QDateTime::currentMSecsSinceEpoch();
    qDebug() << err << fsize << msg;
    if(err < 0) {
        qDebug() << "onDownloadInitResult" << msg;
    }else{
        m_fileSize = fsize;
        qint64 pos = 0;
        if(err == 1) {
            //base on last process and continue get next block.
            pos = m_file->size();
        }
        m_file->seek(pos);
        int percent = (m_fileSize > 0) ? (pos * 100 / m_fileSize) : 0;
        emit progress(percent, pos, m_fileSize);
        m_ftp->downloadStart(pos, jobId);
    }
}

void QKxFtpDownload::onDownloadResult(qint8 err, const QByteArray &data, qint32 jobId)
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
        emit error(err-10, data);
        return;
    }
    m_file->write(data);
    qint64 fsize = m_file->size();
    int percent = (m_fileSize > 0) ? (fsize * 100 / m_fileSize) : 0;
    //qDebug() << "onDownloadResult" << percent << fsize << m_fileSize << err << data.length();
    if(percent != m_percentLast) {
        m_percentLast = percent;
        emit progress(percent, fsize, m_fileSize);
    }
    if(fsize < m_fileSize) {
        m_ftp->downloadRequest(jobId);
        m_file->flush();
    }else if(fsize == m_fileSize) {
        m_ftp->downloadFinish(jobId);
        m_file->close();
        qint64 elapse = QDateTime::currentMSecsSinceEpoch() - m_tmStart;
        int bytePer = elapse > 0 ? m_fileSize * 1000 / elapse : 0;
        qDebug() << "finish task used" << elapse << m_fileSize << bytePer;
        emit finished();
    }else{
        m_ftp->downloadAbort(jobId);
        m_file->close();
        emit error(-4, tr("Bad Size"));
    }
}

void QKxFtpDownload::verifyMd5(qint32 jobId)
{
    if(m_file == nullptr || m_jobId != jobId) {
        return;
    }
    QByteArray buf = m_file->read(MAX_BLOCK_SIZE);
    if(!buf.isEmpty()) {
        m_crypt.addData(buf);
        QMetaObject::invokeMethod(this, "verifyMd5", Qt::QueuedConnection, Q_ARG(qint32, jobId));
    }else {
        QByteArray md5 = m_crypt.result();
        int fsize = m_file->size();
        m_ftp->downloadInit(m_remote.toUtf8(), md5, fsize, jobId);
    }
}
