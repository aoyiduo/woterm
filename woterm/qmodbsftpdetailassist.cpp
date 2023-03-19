/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qmodbsftpdetailassist.h"

#include "qwoglobal.h"
#include "qwossh.h"
#include "qwoidentify.h"
#include "qkxmessagebox.h"

#include <QTimer>

QMoDbSftpDetailAssist::QMoDbSftpDetailAssist(QWidget *parent)
    : QObject(parent)
    , m_widgetParent(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(onTestTimeout()));
}

QMoDbSftpDetailAssist::~QMoDbSftpDetailAssist()
{

}

void QMoDbSftpDetailAssist::verify(const QVariant &v)
{
    QVariantMap dm = v.toMap();
    if(dm.isEmpty()) {
        return;
    }
    HostInfo hi;
    hi.host = dm.value("host").toString();
    hi.user = dm.value("name").toString();
    hi.port = dm.value("port").toInt();
    int itype = dm.value("type").toInt();
    QString path = dm.value("path").toString();
    if(itype == 0) {
        hi.password = dm.value("password").toString();
    }else{
        hi.identityFile = dm.value("identity").toString();
        IdentifyInfo info;
        if(!QWoIdentify::infomation(hi.identityFile.toUtf8(), &info)) {
            return;
        }
        hi.identityContent = info.prvKey;
    }
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));
    m_sftp->start(hi, 11);    
    m_sftp->fileInfo(path, dm);

    m_timer->start(1000 * 10);
}

void QMoDbSftpDetailAssist::onConnectionStart()
{

}

void QMoDbSftpDetailAssist::onConnectionFinished(bool ok)
{
    if(!ok) {
        release();
        QKxMessageBox::information(m_widgetParent, tr("Failure report"), tr("Failed"));
    }
}

void QMoDbSftpDetailAssist::onCommandStart(int type, const QVariantMap &userData)
{

}

void QMoDbSftpDetailAssist::onCommandFinish(int type, const QVariantMap &userData)
{
    QString reason = userData.value("reason").toString();
    if(type == 22) {
        // #define MT_FTP_MKPATH           (22)
        release();
        if(reason == "ok") {
            QKxMessageBox::information(m_widgetParent, tr("Success"), tr("Success to create it."));
        }else{
            QKxMessageBox::information(m_widgetParent, tr("Failure"), tr("Failed to create it, please do it by manual."));
        }
    }else if(type == 21) {
        // #define MT_FTP_FILE_INFO        (21)
        QVariantMap dm = userData.value("fileInfo").toMap();
        if(dm.isEmpty()) {
            if(reason != "fatal") {
                if(QKxMessageBox::information(m_widgetParent,
                                              tr("Failure"),
                                              tr("Failed to open the target path, try to create it?"),
                                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
                    QString path = dm.value("path").toString();
                    m_sftp->mkPath(path, 0x1C0);
                }else{
                    release();
                }
            }else{
                release();
            }
        }else{
            release();
            QString type = dm.value("type").toString();
            if(type != "d") {
                QKxMessageBox::information(m_widgetParent, tr("Failure"), tr("Please check it again and make sure the target path is a directory not a file."));
            }else{
                QKxMessageBox::information(m_widgetParent, tr("Success"), tr("It's a valid path to backup file."));
            }
        }
    }else{
        release();
    }
}

void QMoDbSftpDetailAssist::onErrorArrived(const QString &err, const QVariantMap &userData)
{
    QKxMessageBox::information(m_widgetParent, tr("Error"), err);
    release();
}

void QMoDbSftpDetailAssist::onFinishArrived(int code)
{
    release();
}

void QMoDbSftpDetailAssist::onInputArrived(const QString &title, const QString &prompt, bool visible)
{
    release();
    QKxMessageBox::information(m_widgetParent, tr("Parmeter error"), tr("The account information error"));
}

void QMoDbSftpDetailAssist::onTestTimeout()
{
    release();
    QKxMessageBox::information(m_widgetParent, tr("Error"), tr("Test timeout"));
}

void QMoDbSftpDetailAssist::release()
{
    m_timer->stop();
    if(m_sftp) {
        m_sftp->stop();
        QWoSshFactory::instance()->release(m_sftp);
    }
    m_sftp = nullptr;
}
