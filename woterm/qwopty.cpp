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

#include "qwopty.h"

#include <ptyqt.h>

#include <QProcessEnvironment>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

QWoPty::QWoPty(QObject *parent)
    : QObject(parent)
{

}

QWoPty::~QWoPty()
{

}

bool QWoPty::hasRunning()
{
    return isRunning();
}

bool QWoPty::start(int cols, int rows)
{
    return init(cols, rows);
}

void QWoPty::stop()
{
    onFinished();
}

class QPowerPty : public QWoPty {
private:
    QPointer<IPtyProcess> m_pty;
public:
    explicit QPowerPty(QObject *parent = nullptr)
        : QWoPty(parent) {

    }

    virtual ~QPowerPty() {
        cleanup();
    }

    bool init(int cols, int rows) {
        if(m_pty) {
            return false;
        }
        if(cols <= 0) {
            cols = 40;
        }
        if(rows <= 0) {
             rows = 20;
        }
        IPtyProcess::PtyType ptyType;
        QString shellPath;
#ifdef Q_OS_WIN
        ptyType = IPtyProcess::WinPty;
        QByteArray cmdPath = qgetenv("ComSpec");
        if(cmdPath.isEmpty()) {
            cmdPath = "c:\\Windows\\system32\\cmd.exe";
        }
        if(!QFile::exists(cmdPath)) {
            cmdPath = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
        }
        if(!QFile::exists(cmdPath)) {
            emit errorArrived(tr("Failed to find any command program.").toUtf8());
            emit finishArrived(-1);
            return  false;
        }
        shellPath = cmdPath;
        QString version = QSysInfo::kernelVersion();
        qint32 buildNumber = version.split(".").last().toInt();
        if (buildNumber >= CONPTY_MINIMAL_WINDOWS_VERSION) {
            qDebug() << "Use ConPty instead of WinPty";
            ptyType = IPtyProcess::ConPty;
        }
        ptyType = IPtyProcess::WinPty;
        qDebug() << version << buildNumber << ptyType;
#else
        ptyType = IPtyProcess::UnixPty;
        QStringList programs = {QString::fromUtf8(qgetenv("SHELL")), "/bin/bash", "/bin/sh"};
        QString exec;
        for(int i = 0; i < programs.length(); i++) {
            exec = QStandardPaths::findExecutable(programs.at(i));
            if(!exec.isEmpty()) {
                break;
            }
        }
        shellPath = exec;
#endif
        m_pty = PtyQt::createPtyProcess(ptyType);
        if(!m_pty->startProcess(shellPath, QProcessEnvironment::systemEnvironment().toStringList(), qint16(cols), qint16(rows))
                || !m_pty->lastError().isEmpty()) {
            QString errMsg = m_pty->lastError();
            delete m_pty;
            emit connectionFinished(false);
            emit errorArrived(errMsg.toUtf8());
            emit finishArrived(-1);
            return false;
        }
        QObject::connect(m_pty->notifier(), SIGNAL(readyRead()),  this, SLOT(onReadyRead()));
#ifdef Q_OS_WIN
        QLocalSocket *localSocket = qobject_cast<QLocalSocket *>(m_pty->notifier());
        QObject::connect(localSocket, SIGNAL(disconnected()), this, SLOT(onFinished()));
#else
        QProcess *shellProcess = qobject_cast<QProcess *>(m_pty->notifier());
        QObject::connect(shellProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished()));
#endif
        emit connectionFinished(true);
        return true;
    }

    void write(const QByteArray& buf){
        if(m_pty) {
            m_pty->write(buf);
        }
    }

    void updateSize(int cols, int rows){
        if(cols <= 0 || rows <= 0) {
            return;
        }
        if(m_pty) {
            m_pty->resize(qint16(cols), qint16(rows));
        }
    }

    bool cleanup() {
        IPtyProcess *pty = m_pty;
        if(m_pty) {
            m_pty = nullptr;
            pty->kill();
            delete pty;
            return true;
        }
        return false;
    }

    void onReadyRead() {
        if(m_pty){
            QByteArray all = m_pty->readAll();
            emit dataArrived(all);
        }
    }

    void onFinished(){
        if(m_pty && cleanup()) {
            emit finishArrived(0);
        }
    }

    bool isRunning() {
        return m_pty;
    }
};





QWoPtyFactory::QWoPtyFactory(QObject *parent)
    : QObject(parent)
{

}

QWoPtyFactory::~QWoPtyFactory()
{

}

QWoPtyFactory *QWoPtyFactory::instance()
{
    static QPointer<QWoPtyFactory> factory = new QWoPtyFactory();
    return factory;
}

QWoPty *QWoPtyFactory::create()
{
    return new QPowerPty(this);
}

void QWoPtyFactory::release(QWoPty *obj)
{
    obj->disconnect();
    QObject::connect(obj, SIGNAL(finishArrived(int)), this, SLOT(onFinished(int)), Qt::QueuedConnection);
    if(!obj->hasRunning()) {
        obj->deleteLater();
        return;
    }
    obj->stop();
    m_dels.append(obj);
}

void QWoPtyFactory::onFinished(int)
{
    cleanup();
}

void QWoPtyFactory::onAboutToQuit()
{

}

void QWoPtyFactory::cleanup()
{
    for(QList<QPointer<QWoPty>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
        QWoPty *obj = *iter;
        if(obj == nullptr) {
            iter = m_dels.erase(iter);
            continue;
        }
        if(!obj->hasRunning()) {
            obj->deleteLater();
            iter = m_dels.erase(iter);
            continue;
        }
        obj->stop();
        iter++;
    }
}
