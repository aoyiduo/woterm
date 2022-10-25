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

#include "qwoutils.h"

#include <QObject>
#include<QLayout>
#include<QWidget>
#include <QSpacerItem>
#include <QDebug>
#include <QBoxLayout>
#include <QDataStream>
#include <QByteArray>
#include <QDir>
#include <QCoreApplication>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QProcess>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fcntl.h>
#define myclosesocket  closesocket

#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#define myclosesocket(x)    close(x)
#endif

static ulong LOCAL_IP = inet_addr("127.0.0.1");

void QWoUtils::setLayoutVisible(QLayout *layout, bool visible)
{
    QBoxLayout *box = qobject_cast<QBoxLayout*>(layout);
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);
        QWidget *w = item->widget();
        if(w) {
            w->setVisible(visible);
        }else {
             QSpacerItem *spacer = item->spacerItem();
             if(spacer) {
                 QSize sz = spacer->sizeHint();
                 QSizePolicy policy = spacer->sizePolicy();
                 Qt::Orientations orient = spacer->expandingDirections();
                 if(orient == Qt::Horizontal) {
                     spacer->changeSize(sz.width(), 0, policy.horizontalPolicy(), policy.verticalPolicy());
                 }else if(orient == Qt::Vertical){
                     spacer->changeSize(0, sz.height(), policy.horizontalPolicy(), policy.verticalPolicy());
                 }
             }else{
                 QLayout *layout = item->layout();
                 if(layout) {
                     QWoUtils::setLayoutVisible(layout, visible);
                 }
             }
        }
    }
}

QString QWoUtils::qVariantToBase64(const QVariant& v)
{
    QByteArray buf;
    QDataStream in(&buf, QIODevice::WriteOnly);
    in << v;
    return QString(buf.toBase64());
}

QVariant QWoUtils::qBase64ToVariant(const QString& v)
{
    QByteArray buf = QByteArray::fromBase64(v.toUtf8());
    QDataStream out(buf);
    QVariant data;
    out >> data;
    return data;
}

QStringList QWoUtils::parseCombinedArgString(const QString &program)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < program.size(); ++i) {
        if (program.at(i) == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += program.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && program.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += program.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}


void rc4_init(unsigned char *s, unsigned char *key, unsigned long Len) //初始化函数
{
    int i =0, j = 0;
    unsigned char k[256] = {0};
    unsigned char tmp = 0;
    for (i=0;i<256;i++) {
        s[i] = i;
        k[i] = key[i%Len];
    }
    for (i=0; i<256; i++) {
        j=(j+s[i]+k[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[i]和s[j]
        s[j] = tmp;
    }
}

void rc4_crypt(unsigned char *s, unsigned char *Data, unsigned long Len) //加解密
{
    int i = 0, j = 0, t = 0;
    long k = 0;
    unsigned char tmp;
    for(k=0;k<Len;k++) {
        i=(i+1)%256;
        j=(j+s[i])%256;
        tmp = s[i];
        s[i] = s[j]; //交换s[x]和s[y]
        s[j] = tmp;
        t=(s[i]+s[j])%256;
        Data[k] ^= s[t];
    }
}

QByteArray QWoUtils::rc4(const QByteArray &data, const QByteArray &key)
{
    unsigned char s[256] = {0};
    QByteArray buf(data);
    rc4_init(s, (unsigned char *)key.data(),  key.length());
    rc4_crypt(s, (unsigned char *)buf.data(), buf.length());
    return buf;
}

QString QWoUtils::nameToPath(const QString &name)
{
    return name.toUtf8().toBase64(QByteArray::OmitTrailingEquals|QByteArray::Base64UrlEncoding);
}

QString QWoUtils::pathToName(const QString &path)
{
    return QByteArray::fromBase64(path.toUtf8(), QByteArray::OmitTrailingEquals|QByteArray::Base64UrlEncoding);
}

int QWoUtils::versionToLong(const QString &ver)
{
    QStringList parts = ver.split('.');
    if(parts.length() == 3) {
        int ver_major = parts.at(0).toInt() * 100000;
        int ver_minor = parts.at(1).toInt() * 1000;
        int ver_minor2 = parts.at(2).toInt();
        return ver_major + ver_minor + ver_minor2;
    }else if(parts.length() == 2){
        int ver_major = parts.at(0).toInt() * 100000;
        int ver_minor = parts.at(1).toInt() * 100;
        int ver_minor2 = 0;
        return ver_major + ver_minor + ver_minor2;
    }
    return 0;
}

QByteArray QWoUtils::toWotermStream(const QByteArray &data)
{
    QByteArray key("woterm.2019");
    if(data.startsWith("woterm:")) {
        return data;
    }
    QByteArray buf = rc4(data, key);
    buf.insert(0, "woterm:");
    return buf;
}

QByteArray QWoUtils::fromWotermStream(const QByteArray &data)
{
    QByteArray key("woterm.2019");
    if(data.startsWith("woterm:")) {
        QByteArray buf(data);
        buf = buf.remove(0,  7);
        return rc4(buf, key);
    }
    return data;
}

int QWoUtils::hasDesktopShortcut(bool everyone)
{
    QDir dir = QDir(everyone ? "/usr/share/applications" : QDir::homePath() + "/.local/share/applications");
    if(!dir.exists()) {
        return -99;
    }
    QString execPath = QCoreApplication::applicationFilePath();
    QString appName = QCoreApplication::applicationName();
    QString path = dir.absoluteFilePath(QString("%1.desktop").arg(appName));
    QFile f(path);
    if(!f.open(QFile::ReadOnly|QFile::Text)) {
        QFileDevice::FileError err = f.error();
        if(err & (QFileDevice::ReadError | QFileDevice::OpenError)) {
            return -1;
        }
        return -2;
    }
    while(!f.atEnd()) {
        QByteArray line = f.readLine();
        if(line == "Exec="+execPath) {
            return 1;
        }
    }
    return 0;
}

int QWoUtils::createDesktopShortcut(bool everyone, const QString& name, const QString& desc, const QString& iconPath)
{
    QDir dir = QDir(everyone ? "/usr/share/applications" : QDir::homePath() + "/.local/share/applications");
    if(!dir.exists()) {
        return -99;
    }
    QString execPath = QCoreApplication::applicationFilePath();
    QString appName = QCoreApplication::applicationName();
    QString path = dir.absoluteFilePath(QString("%1.desktop").arg(appName));
    QStringList all;
    all.append("[Desktop Entry]");
    all.append("Terminal=false");
    all.append("Categories=Application;Network;Security;Qt;");
    all.append("Type=Application");
    all.append("Exec="+execPath);
    all.append("Name="+name);
    all.append("GenericName="+desc);
    all.append("Icon[en_US]="+iconPath);
    all.append("Comment[en_US]="+desc);

    QFile f(path);
    if(!f.open(QFile::WriteOnly|QFile::Text)) {
        QFileDevice::FileError err = f.error();
        if(err & (QFileDevice::WriteError | QFileDevice::OpenError)) {
            return -1;
        }
        return -2;
    }
    QString buf = all.join("\n");
    int nWrite = f.write(buf.toLocal8Bit());
    f.close();
    int code = QProcess::execute("chmod a+x " + path);
    qDebug() << "Desktop Entry chmod code" << code;
    return nWrite;
}

void QWoUtils::removeDesktopShortcut(bool everyone)
{
    QDir dir = QDir(everyone ? "/usr/share/applications" : QDir::homePath() + "/.local/share/applications");
    if(!dir.exists()) {
        return;
    }
    QString appName = QCoreApplication::applicationName();
    QString path = dir.absoluteFilePath(QString("%1.desktop").arg(appName));
    QFile::remove(path);
}

//suid, sgid, sticky_bit | user_read, user_write, user_execute | group_read, group_write, group_execute | others_read, others_write, others_execute
#define FILE_PERMISSION_SUID            (1 << 11)
#define FILE_PERMISSION_SGID            (1 << 10)
#define FILE_PERMISSION_STICKY_BIT      (1 << 9)
#define FILE_PERMISSION_USER_READ       (1 << 8)
#define FILE_PERMISSION_USER_WRITE      (1 << 7)
#define FILE_PERMISSION_USER_EXECUTE    (1 << 6)
#define FILE_PERMISSION_GROUP_READ      (1 << 5)
#define FILE_PERMISSION_GROUP_WRITE     (1 << 4)
#define FILE_PERMISSION_GROUP_EXECUTE   (1 << 3)
#define FILE_PERMISSION_OTHER_READ      (1 << 2)
#define FILE_PERMISSION_OTHER_WRITE     (1 << 1)
#define FILE_PERMISSION_OTHER_EXECUTE   (1)
QByteArray QWoUtils::filePermissionToText(int type, int flag)
{
    QByteArray buf;
    switch (type) {
    case 1:
        buf.append('-');
        break;
    case 2:
        buf.append('d');
        break;
    case 3:
        buf.append('l');
        break;
    default:
        buf.append('0'+type);
        break;
    }
    buf.append(flag&FILE_PERMISSION_USER_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_USER_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_USER_EXECUTE ? 'x':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_GROUP_EXECUTE ? 'x':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_READ ? 'r':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_WRITE ? 'w':'-');
    buf.append(flag&FILE_PERMISSION_OTHER_EXECUTE ? 'x':'-');
    return buf;
}

int QWoUtils::getAddrInfos(const char *host, int port, addrinfo **ai)
{
    const char *service = nullptr;
    struct addrinfo hints;
    char s_port[10];

    memset(&hints, 0, sizeof(hints));

    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (port == 0) {
        hints.ai_flags = AI_PASSIVE;
    } else {
        qsnprintf(s_port, sizeof(s_port), "%hu", (unsigned short)port);
        service = s_port;
#ifdef AI_NUMERICSERV
        hints.ai_flags = AI_NUMERICSERV;
#endif
    }

    return getaddrinfo(host, service, &hints, ai);
}

void QWoUtils::freeAddrInfos(addrinfo *addr)
{
    freeaddrinfo(addr);
}

bool QWoUtils::isAgain(int err)
{
#ifdef Q_OS_WIN
    return err == EAGAIN || err == EWOULDBLOCK || err == WSAEWOULDBLOCK;
#else
    return err == EAGAIN;
#endif
}

void QWoUtils::setSocketNonBlock(int sock, bool on)
{
#ifdef Q_OS_WIN
    ulong nonblocking = on ? 1 : 0;
    ioctlsocket(sock, FIONBIO, &nonblocking);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if(on) {
        fcntl(sock, F_SETFL, flags|O_NONBLOCK);
    }else{
        fcntl(sock, F_SETFL, flags&~O_NONBLOCK);
    }
#endif
}

void QWoUtils::setSocketNoDelay(int sock, bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));
}

void QWoUtils::setSocketReusable(int sock, bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
}

void QWoUtils::setSocketKeepAlive(int sock, bool on)
{
    int flag = on ? 1 : 0;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(flag));
}

bool QWoUtils::createPair(int server, ushort port, int fd[])
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = LOCAL_IP;
    addr.sin_port = htons(port);
    int fd1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(::connect(fd1, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        myclosesocket(fd1);
        return false;
    }
    int fd2 = accept(int(server), nullptr, nullptr);
    fd[0] = int(fd1);
    fd[1] = int(fd2);
    return true;
}

bool QWoUtils::createPair2(ushort basePort, int fd[])
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    for(ushort i = 0; i < 100; i++){
        addr.sin_port = htons(basePort + i);
        if(bind(server, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
            continue;
        }
        if(listen(server, 5) == -1) {
            myclosesocket(server);
            return 0;
        }
        int fd1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(::connect(fd1, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
            myclosesocket(fd1);
            myclosesocket(server);
            return false;
        }
        int fd2 = accept(server, nullptr, nullptr);
        fd[0] = int(fd1);
        fd[1] = int(fd2);
        myclosesocket(server);
        return true;
    }
    myclosesocket(server);
    return false;
}

ushort QWoUtils::listenLocal(int server, ushort basePort)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = LOCAL_IP;
    for(ushort i = 0; i < 100; i++){
        addr.sin_port = htons(basePort + i);
        if(bind(server, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
            continue;
        }
        if(listen(server, 5) == -1) {
            return 0;
        }
        return basePort + i;
    }
    return 0;
}

int QWoUtils::socketError()
{
#ifdef Q_OS_WIN
    int err = WSAGetLastError();
    return err;
#else
    return errno;
#endif
}

int QWoUtils::xRecv(int sock, char *buf, int len, int flag)
{
    int left = len;
    while(left > 0) {
        int n = ::recv(sock, buf, left, flag);
        if(n < 0) {
            if(QWoUtils::isAgain(socketError())) {
                return len - left;
            }
            return n;
        }
        if(n == 0) {
            return len - left;
        }
        buf += n;
        left -= n;
    }
    return len;
}

int QWoUtils::xSend(int sock, char *buf, int len, int flag)
{
    int left = len;
    while(left > 0) {
        int n = ::send(sock, buf, left, flag);
        if(n < 0) {
            if(QWoUtils::isAgain(socketError())) {
                return len - left;
            }
            return n;
        }
        if(n == 0) {
            return len - left;
        }
        buf += n;
        left -= n;
    }
    return len;
}

int QWoUtils::gid()
{
    static int myid = 1000;
    return myid++;
}


bool QWoUtils::isRootUser()
{
    int uid = 0;
#ifdef Q_OS_LINUX
    uid = getuid();
#endif
    return uid == 0;
}

QString QWoUtils::loginUser()
{
    QProcess proc;
    QStringList args;
    args.append("-c");
    args.append("logname");
    proc.start("/bin/sh", args);
    if(!proc.waitForStarted(3000)) {
        return "";
    }
    if(!proc.waitForFinished(3000)) {
        return "";
    }
    QByteArray out = proc.readAllStandardOutput();
    return out;
}

bool QWoUtils::openself(const QString& type, const QString& target, bool pkexec)
{
    QStringList args(QCoreApplication::applicationFilePath());
    args.append("--target="+target);
    args.append("--type="+type);
    QString display = qgetenv("DISPLAY");
    QString xauth = qgetenv("XAUTHORITY");
    QString expath = qgetenv("PATH");
    QString ldpath = qgetenv("LD_LIBRARY_PATH");
    QString conf = qgetenv("WOTERM_DATA_PATH");
    QStringList envs;
    envs.append(QString("DISPLAY=\"%1\"").arg(display));
    envs.append(QString("XAUTHORITY=\"%1\"").arg(xauth));
    envs.append(QString("WOTERM_DATA_PATH=\"%1\"").arg(conf));
    envs.append(QString("PATH=%1").arg(expath));
    envs.append(QString("LD_LIBRARY_PATH=%1").arg(ldpath));
    QString program;
    if(pkexec) {
        program = QString("pkexec env %1 %2").arg(envs.join(QChar::Space)).arg(args.join(QChar::Space));
    }else{
        program = args.join(QChar::Space);
    }
    return QProcess::startDetached(program);
}

int QWoUtils::parseVersion(const QString &ver)
{
    int verInt = 0;
    QStringList vers = ver.split('.');
    if(vers.count() == 1) {
        int major = vers.at(0).toInt() * 1000000;
        verInt = major;
    }else if(vers.count() == 2) {
        int major = vers.at(0).toInt() * 1000000;
        int minor = vers.at(1).toInt() * 1000;
        verInt = major + minor;
    }else if(vers.count() == 3) {
        int major = vers.at(0).toInt() * 1000000;
        int minor = vers.at(1).toInt() * 1000;
        int patch = vers.at(2).toInt();
        verInt = major + minor + patch;
    }
    return verInt;
}

