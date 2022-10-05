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

#include "qkxmacaddress.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTcpSocket>
#include <QProcess>
#include <QTimer>
#include <QNetworkProxy>


QKxMACAddress::QKxMACAddress(QObject *parent)
    : QObject(parent)
{
}

void QKxMACAddress::start(const QString &host, quint16 port)
{
    m_host = host;
    m_port = port;
    restart();
}

void QKxMACAddress::restart()
{
    clean();
    m_hasSendResult = false;
    m_ipToMac = getCandidate();
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void QKxMACAddress::onError()
{
    QTcpSocket *tcp = qobject_cast<QTcpSocket*>(sender());
    quint16 port = tcp->localPort();
    QString err = tcp->errorString();
    sendError(err);
}

void QKxMACAddress::onConnected()
{
    QTcpSocket *tcp = qobject_cast<QTcpSocket*>(sender());
    quint16 port = tcp->localPort();
    QProcess *proc = new QProcess(this);
    m_proc = proc;
    QObject::connect(proc, SIGNAL(finished(int)), this, SLOT(onNetstatResult()));

#if defined(Q_OS_WIN)
    QString cmd = QString("/c netstat -ano|find \"%1\"|find \"%2\"").arg(port).arg(m_port);
    qDebug() << "mac check" << cmd;
    proc->setNativeArguments(cmd);
    proc->start("cmd.exe");
#elif defined(Q_OS_MAC)
    QStringList args;
    args << "-c" << QString("netstat -nat|grep %1|grep %2").arg(port).arg(m_port);
    qDebug() << "mac check" << args;
    proc->start("/bin/bash", args);
#else
    QStringList args;
    args << "-c" << QString("netstat -ntp|grep %1|grep %2").arg(port).arg(m_port);
    qDebug() << "mac check" << args;
    proc->start("/bin/bash", args);
#endif
}

void QKxMACAddress::onDisconnected()
{
    QTcpSocket *tcp = qobject_cast<QTcpSocket*>(sender());
    tcp->deleteLater();
}

void QKxMACAddress::onNetstatResult()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    QByteArray buf = proc->readAllStandardOutput();
    for(auto it = m_ipToMac.begin(); it != m_ipToMac.end(); it++) {
        QString ip = it.key();
        if(buf.contains(ip.toLatin1())) {
            QString mac = it.value();
            m_macUsed = mac;
            sendResult(mac);
            return;
        }
    }
}

void QKxMACAddress::onTimeout()
{
    sendError("timeouted");
}

void QKxMACAddress::init()
{
    if(m_ipToMac.size() > 1) {
        QTcpSocket *tcp = new QTcpSocket();
        tcp->setProxy(QNetworkProxy::NoProxy);
        m_tcp = tcp;
        QObject::connect(tcp, SIGNAL(connected()), this, SLOT(onConnected()));
        QObject::connect(tcp, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        QObject::connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError()));
        tcp->connectToHost(m_host, m_port);

        QTimer *timer = new QTimer(this);
        m_timer = timer;
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        timer->setSingleShot(true);
        timer->start(1000 * 5);
    }else if(m_ipToMac.size() == 1){
        QString mac = m_ipToMac.first();
        sendResult(mac);
    }else{
        sendError("no active network.");
    }
}

QMap<QString, QString> QKxMACAddress::getCandidate()
{
    QMap<QString, QString> macs;
    QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    for(int i = 0; i < all.count(); i++) {
        QNetworkInterface ni = all.at(i);
        if(!(ni.flags().testFlag(QNetworkInterface::IsUp)
                && ni.flags().testFlag(QNetworkInterface::IsRunning))) {
            qDebug() << "not need" << ni.hardwareAddress();
            continue;
        }
        QString mac = ni.hardwareAddress().toUpper();
        if(mac.isEmpty()) {
            qDebug() << "not need" << ni.hardwareAddress();
            continue;
        }
        QList<QNetworkAddressEntry> addrs = ni.addressEntries();
        for(int i = 0; i < addrs.length(); i++) {
            QHostAddress addr = addrs.at(i).ip();
            bool ok = false;
            addr.toIPv4Address(&ok);
            if(ok) {
                QString ip = addr.toString();
                if(ip.startsWith("127.0.0.")) {
                    continue;
                }
                qDebug() << ip << mac;
                macs.insert(ip, mac);
            }
        }
    }
    return macs;
}

void QKxMACAddress::clean()
{
    if(m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
    }
    if(m_tcp) {
        m_tcp->close();
        m_tcp->deleteLater();
    }
    if(m_proc) {
        m_proc->close();
        m_proc->deleteLater();
    }
}

void QKxMACAddress::sendError(const QString &err)
{
    if(m_hasSendResult) {
        return;
    }
    m_hasSendResult = true;

    emit errorArrived(err);
    clean();
    qDebug() << "errorArrived" << err;
}

void QKxMACAddress::sendResult(const QString &mac)
{
    if(m_hasSendResult) {
        return;
    }
    m_macUsed = mac;
    m_hasSendResult = true;
    emit macArrived(mac);
    clean();
    qDebug() << "macArrived" << mac;
}
