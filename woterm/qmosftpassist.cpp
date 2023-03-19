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

#include "qmosftpassist.h"

#include "qwossh.h"
#include "qwosetting.h"
#include "qmosftpremotemodel.h"
#include "qkxmessagebox.h"
#include "qwosshconf.h"
#include "qmosftptransferassist.h"

QMoSftpAssist::QMoSftpAssist(const QString& target, int gid, QMoSftpRemoteModel *model, QWidget* widget, QObject *parent)
    : QObject(parent)
    , m_target(target)
    , m_gid(gid)
    , m_model(model)
    , m_widget(widget)
{
    QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
}

void QMoSftpAssist::openHome()
{
    QVariantMap dm;
    dm.insert("path", "~");
    m_sftp->openDir("~", dm);
}

void QMoSftpAssist::openDir(const QStringList &paths)
{
    QVariantMap dm;
    dm.insert("paths", paths);
    m_sftp->openDir(paths, dm);
}

void QMoSftpAssist::openDir(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->openDir(path, dm);
}

void QMoSftpAssist::mkDir(const QString &path, int mode)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->mkDir(path, mode, dm);
}

void QMoSftpAssist::rmDir(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->rmDir(path, dm);
}

void QMoSftpAssist::unlink(const QString &path)
{
    QVariantMap dm;
    dm.insert("path", path);
    m_sftp->unlink(path, dm);
}

void QMoSftpAssist::onCommandStart(int type, const QVariantMap &userData)
{

}

void QMoSftpAssist::onCommandFinish(int type, const QVariantMap &userData)
{
    QString reason = userData.value("reason").toString();
    if(reason == "fatal") {
        release();
    }else if(reason == "error") {
        QString err = userData.value("errorString").toString();
        if(!err.isEmpty()) {
            QKxMessageBox::warning(m_widget, tr("Error"), err, QMessageBox::Ok);
        }
    }
}

void QMoSftpAssist::onConnectionStart()
{

}

void QMoSftpAssist::onConnectionFinished(bool ok)
{
    if(!ok) {
        onFinishArrived(-1);
    }
}

void QMoSftpAssist::onFinishArrived(int code)
{
    if(m_dlgConfirm) {
        return;
    }
    m_dlgConfirm = new QKxMessageBox(QMessageBox::Question, tr("Connection error"), tr("Continue to reconnect to this session?"), QMessageBox::Yes|QMessageBox::No, m_widget);
    if(m_dlgConfirm->exec() == QMessageBox::Yes) {
        QMetaObject::invokeMethod(this, "reconnect", Qt::QueuedConnection);
    }else{
        emit closeArrived();
    }
    m_dlgConfirm->deleteLater();
}

void QMoSftpAssist::onErrorArrived(const QString &err, const QVariantMap &userData)
{
    QKxMessageBox::warning(m_widget, tr("Error"), err, QMessageBox::Ok);
}

void QMoSftpAssist::onPasswordArrived(const QString &host, const QByteArray &pass)
{
    if(m_savePassword) {
        QWoSshConf::instance()->updatePassword(host, pass);
    }
}

void QMoSftpAssist::onPasswordInputResult(const QString &pass, bool isSave)
{
    m_savePassword = isSave;
    if(m_sftp) {
        m_sftp->setInputResult(pass);
    }
}

void QMoSftpAssist::reconnect()
{
    m_sftp = QWoSshFactory::instance()->createSftp();
    QObject::connect(m_sftp, SIGNAL(dirOpen(QString,QVariantList,QVariantMap)), m_model, SLOT(onDirOpen(QString,QVariantList,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(connectionFinished(bool)), this, SLOT(onConnectionFinished(bool)));
    QObject::connect(m_sftp, SIGNAL(connectionStart()), this, SLOT(onConnectionStart()));
    QObject::connect(m_sftp, SIGNAL(finishArrived(int)), this, SLOT(onFinishArrived(int)));
    QObject::connect(m_sftp, SIGNAL(errorArrived(QString,QVariantMap)), this, SLOT(onErrorArrived(QString,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(passwordArrived(QString,QByteArray)), this, SLOT(onPasswordArrived(QString,QByteArray)));
    QObject::connect(m_sftp, SIGNAL(inputArrived(QString,QString,bool)), this, SIGNAL(inputArrived(QString,QString,bool)));

    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SLOT(onCommandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SLOT(onCommandFinish(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandStart(int,QVariantMap)), this, SIGNAL(commandStart(int,QVariantMap)));
    QObject::connect(m_sftp, SIGNAL(commandFinish(int,QVariantMap)), this, SIGNAL(commandFinish(int,QVariantMap)));

    m_sftp->start(m_target, m_gid);

    QString path = QWoSetting::value(QString("sftp/lastPathRemote:%1").arg(m_target), "").toString();
    QStringList paths;
    if(!path.isEmpty()) {
        paths.append(path);
    }
    paths.append("~");
    openDir(paths);
}

void QMoSftpAssist::release()
{
    if(m_sftp) {
        m_sftp->stop();
        QWoSshFactory::instance()->release(m_sftp);
        m_sftp = nullptr;
    }
}
