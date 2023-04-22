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

#include "qworlogin.h"

#include "qwoutils.h"
#include "qwosshconf.h"

#include <QMutexLocker>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fcntl.h>
#define myclosesocket  closesocket
typedef SOCKET socket_t;
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
typedef int socket_t;
#define myclosesocket(x)    close(x)
#endif

#define MT_CONNECT          (1)
#define MT_PTYSIZE          (2)
#define MT_PTYDATA          (3)
#define MT_WAITUP           (4)
#define MT_INPUT            (5)
#define MT_CTRL             (6)
#define MT_EXIT             (0x7f)
struct RLoginMsg {
    uchar type;
    QByteArray data;
};


QRLoginClient::QRLoginClient(const TargetInfo& ti, QObject *parent)
    : QThread(parent)
    , m_ti(ti)
{
    m_cacheCount = 0;
    QObject::connect(this, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(this, SIGNAL(internalDataArrived(QByteArray)), this, SLOT(onInternalDataArrived(QByteArray)));
}

QRLoginClient::~QRLoginClient()
{
    onFinished();
}


bool QRLoginClient::init(int server, ushort port)
{
    int fd[2];
    if(!QWoUtils::createPair(socket_t(server), port, fd)) {
        return false;
    }
    m_msgRead = fd[0];
    m_msgWrite = fd[1];
    QWoUtils::setSocketNoDelay(fd[0], true);
    QWoUtils::setSocketNoDelay(fd[1], true);
    m_server = server;
    m_port = port;
    return true;
}

void QRLoginClient::stop()
{
    for(int i = 0; i < 50; i++) {
        push(MT_EXIT);
    }
    qDebug() << "left" << left;
}

#define MAX_CACHE   (30)
void QRLoginClient::emitDataArrived(const QByteArray &buf)
{
    m_cacheCount++;
    if(m_cacheCount > MAX_CACHE) {
        int cnt = m_cacheCount / MAX_CACHE;
        if(cnt < 10){
            QThread::msleep(cnt * 5);
        }else if(cnt < 30) {
            QThread::msleep(cnt * 10);
        }else if(cnt < 60) {
            QThread::msleep(cnt * 20);
        }else{
            QThread::msleep(cnt * 30);
        }
    }
#if 0
    qDebug() << "emitCount:" << m_cacheCount;
#endif
    emit internalDataArrived(buf);
}

void QRLoginClient::onFinished()
{
    if(m_msgRead > 0) {
        myclosesocket(m_msgRead);
        m_msgRead = 0;
    }
    if(m_msgWrite > 0) {
        myclosesocket(m_msgWrite);
        m_msgWrite = 0;
    }
}

void QRLoginClient::onInternalDataArrived(const QByteArray &buf)
{
    m_cacheCount--;
    emit dataArrived(buf);
}

void QRLoginClient::handleRead(const QByteArray &data)
{
    QByteArray buf = data;
#if 0
    int cnt = buf.length();
    for(int i = 0; i < cnt; i++) {
        if(i + 3 < cnt && uchar(buf.at(i)) == 0x18 && uchar(buf.at(i+1)) == 0x69 && uchar(buf.at(i+2)) == 0xfe && uchar(buf.at(i+3)) == 0x4e) {
            QFile f("maybe.log");
            f.open(QFile::Append);
            f.write(data);
            f.write("\r\n---------------\r\n");
            f.close();
        }
    }
#endif
    emitDataArrived(buf);
}

void QRLoginClient::handleError(const QString &err)
{
    emit errorArrived(err.toUtf8());
}

bool QRLoginClient::handleInput(const QString &prompt, QByteArray &result, bool visble)
{
    emit inputArrived(m_ti.name, prompt, visble);
    fd_set fds;
    while(1) {
        uchar t;
        FD_ZERO(&fds);
        FD_SET(m_msgRead, &fds);
        int n = select(m_msgRead+1, &fds, nullptr, nullptr, nullptr);
        if(QWoUtils::xRecv(m_msgRead, (char*)&t, 1) <= 0) {
            return false;
        }
        if(takeOne(MT_EXIT, result)) {
            return false;
        }
        if(takeOne(MT_INPUT, result)) {
            return true;
        }
    }
}

void QRLoginClient::setInputResult(const QString &pass)
{
    push(MT_INPUT, pass.toUtf8());
}

void QRLoginClient::run()
{
    m_codeExit = running();
    qDebug() << "run exit now" << m_ti.name << m_ti.host;
}

int QRLoginClient::running()
{
    return -1;
}

void QRLoginClient::push(uchar type, const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    RLoginMsg tmp;
    tmp.type = type;
    tmp.data = data;
    m_queue.append(tmp);
    //qDebug() <<  "push" << m_queue.length() << type << data;
    ::send(m_msgWrite, (char*)&type, 1, 0);
}

bool QRLoginClient::pop(uchar &type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    if(m_queue.isEmpty()) {
        return false;
    }
    RLoginMsg tmp = m_queue.takeFirst();
    type = tmp.type;
    data = tmp.data;
    //qDebug() <<  "pop" << m_queue.length() << type << data;
    return true;
}

bool QRLoginClient::takeOne(uchar type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    for(int i = 0; i < m_queue.length(); i++) {
        RLoginMsg &msg = m_queue[i];
        if(msg.type == type) {
            data.swap(msg.data);
            m_queue.removeAt(i);
            return true;
        }
    }
    return false;
}

class QRLoginPtyClient : public QRLoginClient
{
private:
    socket_t m_fd;
    int m_flagEcho;
    int m_flagExit;
    int m_colsLast;
    int m_rowsLast;
public:
    QRLoginPtyClient(const TargetInfo &ti, QObject *parent)
        : QRLoginClient(ti, parent) {
        m_fd = -1;
        m_flagEcho = 1;
        m_flagExit = 0;        
    }
    ~QRLoginPtyClient() {
        cleanup();
    }

    void start(int cols, int rows) {
        m_colsLast = cols;
        m_rowsLast = rows;
        QThread::start();
    }

    void write(const QByteArray &buf) {
        push(MT_PTYDATA, buf);
    }

    void updateSize(int cols, int rows) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << cols << rows;
        m_colsLast = cols;
        m_rowsLast = rows;
        push(MT_PTYSIZE, buf);
    }

    void sendControl(int c) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << c;
        push(MT_CTRL, buf);
    }

private:
    void cleanup(){
        if(m_fd > 0) {
            myclosesocket(m_fd);
            m_fd = 0;
        }
    }

    void rlogin_winsize(int cols, int rows) {
        char b[12] = { '\xFF', '\xFF', 0x73, 0x73, 0, 0, 0, 0, 0, 0, 0, 0 };
        b[6] = (char)(short(cols) >> 8);
        b[7] = (char)(short(cols) & 0xff);
        b[4] = (char)(short(rows) >> 8);
        b[5] = (char)(short(rows) & 0xff);
        QWoUtils::xSend(m_fd, b, 12);
    }

    int ctrl(int x) {
        // 0x40: '@'
        // ctrl+A~Z, [0x41~0x5f]
        return '@' + x;
    }

    void rlogin_special(int c) {
        QWoUtils::xSend(m_fd, (char*)&c, 1);
    }

    int tryToConnect(struct addrinfo *ai)
    {
        /*
         * Rlogin only: bind to a "privileged" port (between 512 and
         * 1023, inclusive).
         *
         * sysctl -w net.ipv4.ip_unprivileged_port_start=80.
         *
         */
        int bindPermission = 0;
        int bindError = 0;
        int connError = 0;
        qint64 seed = QDateTime::currentSecsSinceEpoch();
        qDebug() << "make random seed" << seed;
        qsrand(seed);
        int rid = qAbs(qrand()) % 512;
        rid = rid + 511;
        for (int i = rid; i >= rid - 3; i--) {
            char local_port[10];
            struct addrinfo hints = {0};
            struct addrinfo * local_address;
            qsnprintf(local_port, sizeof(local_port), "%d", i);
            hints.ai_family = ai->ai_family;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            qDebug() << "valid local port for bind:" << i;
            int err = getaddrinfo(NULL, local_port, &hints, &local_address);
            if (err != 0) {
                continue;
            }
            socket_t fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            err = ::bind(fd, local_address->ai_addr, local_address->ai_addrlen);
            if(err != 0) {
                if(QWoUtils::socketError() == EACCES) {
                    bindPermission++;
                }else{
                    bindError++;
                }
                myclosesocket(fd);
                continue;
            }
            err = ::connect(fd, ai->ai_addr, ai->ai_addrlen);
            if(err != 0){
                connError++;
                myclosesocket(fd);
                continue;
            }
            freeaddrinfo(local_address);
            return fd;
        }
        if(bindPermission > 0) {
            handleError(tr("Failed to bind to local port for no permission, switch to root user and try it again?"));
        } else if(bindError > 0) {
            handleError(tr("Failed to bind to local port for unknow reasion"));
        }else if(connError > 0) {
            handleError(tr("Failed to connect to remote server"));
        }
        return -1;
    }

    void rlogin_init() {
        QByteArray buf;
        buf.append(char(0));
        char localusername[] = "local";
        buf.append(localusername, sizeof(localusername));
        buf.append(m_ti.user);
        buf.append(char(0));
        char tty[] = "xterm/38400";
        buf.append(tty, sizeof(tty));
        QWoUtils::xSend(m_fd, buf.data(), buf.length());
    }


    void run() {
        m_codeExit = running();
        qDebug() << "run exit now" << m_ti.name << m_ti.host;
        cleanup();
    }

    int running() {
        struct addrinfo *ai = nullptr;
        struct addrinfo *itr = nullptr;
        int err = QWoUtils::getAddrInfos(m_ti.host, m_ti.port, &ai);
        if(err != 0) {
            return -1;
        }

        m_fd = -1;
        for (itr = ai; itr != nullptr; itr = itr->ai_next) {
            /* create socket */            
            socket_t fd = tryToConnect(itr);
            if(err < 0) {
                continue;
            }
            m_fd = fd;
            break;
        }
        QWoUtils::freeAddrInfos(ai);
        if(m_fd < 0) {
            return -2;
        }
        fd_set rfds,efds;
        QWoUtils::setSocketNoDelay(m_msgRead, true);
        QWoUtils::setSocketNonBlock(m_msgRead, true);
        QWoUtils::setSocketNoDelay(m_fd, true);
        QWoUtils::setSocketNonBlock(m_fd, true);
        QWoUtils::setSocketReusable(m_fd, true);
        m_flagExit = 0;
        rlogin_init();
        bool canResize = false;
        int byteCount = 0;
        while(!m_flagExit) {
            timeval tm={3,0};
            FD_ZERO(&rfds);
            FD_SET(m_msgRead, &rfds);
            FD_SET(m_fd, &rfds);
            int fdmax = int(m_fd);
            if(fdmax < m_msgRead) {
                fdmax = m_msgRead;
            }
            efds = rfds;
            int n = select(fdmax+1, &rfds, nullptr, &efds, &tm);
            if(n == 0) {
                continue;
            }
            if(n < 0) {
                return -3;
            }
            if(FD_ISSET(m_fd, &efds)) {
                QByteArray buf(10240, Qt::Uninitialized);
                int cnt = QWoUtils::xRecv(m_fd, buf.data(), 1, MSG_OOB);
                if( cnt == 1){
                    uchar ch = buf.at(0);
                    if(ch == 0x80) {
                        rlogin_winsize(m_colsLast, m_rowsLast);
                        canResize = true;
                    }else if (ch == 0x02) {
                        /*
                         * Discard unprocessed screen data
                         */
                    } else if (ch == 0x10) {
                        /*
                         * Switch to "raw" mode (pass XON/XOFF to remote side)
                         */
                    } else if (ch == 0x20) {
                        /*
                         * Switch to "cooked" mode (handle XON/XOFF locally)
                         */
                    }
                }else{
                    return -1;
                }
            }
            if(FD_ISSET(m_msgRead, &rfds)) {
                char type;
                if(QWoUtils::xRecv(m_msgRead, (char*)&type, 1) > 0) {
                    uchar type;
                    QByteArray data;
                    while(pop(type, data)) {
                        if(type == MT_EXIT) {
                            qDebug() << "running exit now" << m_ti.name << m_ti.host;
                            return 0;
                        }else if(type == MT_PTYDATA) {
#if 0
                            QFile f("raw.log");
                            if(f.open(QFile::Append)) {
                                f.write(data);
                                f.write("\r\n---------------------------\r\n");
                                f.close();
                            }
#endif
                            QWoUtils::xSend(m_fd, data.data(), data.length());
                        }else if(type == MT_PTYSIZE) {
                            int cols, rows;
                            QDataStream buf(data);
                            buf >> cols >> rows;
                            if(canResize){
                                rlogin_winsize(cols, rows);
                            }
                        }else if(type == MT_CTRL) {
                            int c;
                            QDataStream buf(data);
                            buf >> c;
                            rlogin_special(c);
                        }
                    }
                }else{
                    return -1;
                }
            }

            if(FD_ISSET(m_fd, &rfds)) {
                QByteArray buf(10240, Qt::Uninitialized);
                int cnt = QWoUtils::xRecv(m_fd, buf.data(), buf.capacity());
                if( cnt > 0){
                    if(buf.at(0) == '\0') {
                        //must be zero..when first byte.
                    }
                    if(byteCount < 300) {
                        byteCount += cnt;
                    }else{
                        canResize = true;
                    }
                    buf.resize(cnt);
                    handleRead(buf);
                }else{
                    return -1;
                }
            }
        }
        return 0;
    }
};

QWoRLogin::QWoRLogin(QObject *parent)
    : QObject(parent)
{

}

QWoRLogin::~QWoRLogin()
{
    cleanup();
}

bool QWoRLogin::hasRunning()
{
    if(m_pty == nullptr) {
        return false;
    }
    return m_pty->isRunning();
}

bool QWoRLogin::start(const QString &host, int cols, int rows)
{
    if(m_pty && m_pty->isRunning()) {
        return false;
    }
    if(!init(host)) {
        return false;
    }
    m_pty->start(cols, rows);
    return true;
}

void QWoRLogin::stop()
{
    if(m_pty) {
        m_pty->stop();
    }
}

void QWoRLogin::setInputResult(const QString &pass)
{

}

void QWoRLogin::write(const QByteArray &buf)
{
    if(m_pty) {
        m_pty->write(buf);
    }
}

void QWoRLogin::updateSize(int cols, int rows)
{
    if(m_pty) {
        m_pty->updateSize(cols, rows);
    }
}

void QWoRLogin::sendControl(char c)
{
    m_pty->sendControl(c);
}

void QWoRLogin::onInputArrived(const QString &host, const QString &prompt, bool show)
{
    QRLoginClient *cli = qobject_cast<QRLoginClient*>(sender());
    m_input = cli;
    emit inputArrived(host, prompt, show);
}

void QWoRLogin::onFinished()
{
    QRLoginClient *cli = qobject_cast<QRLoginClient*>(sender());
    if(cli) {
        int code = cli->exitCode();
        emit finishArrived(code);
    }
}

bool QWoRLogin::init(const QString &host)
{
    HostInfo hi = QWoSshConf::instance()->find(host);
    if(!hi.isValid()) {
        return false;
    }
    if(m_listenSocket > 0) {
        myclosesocket(socket_t(m_listenSocket));
    }
    m_listenSocket = 0;
    m_listenPort = 0;
    socket_t server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    ushort port = QWoUtils::listenLocal(server, 20507);
    if(port == 0) {
        myclosesocket(server);
        return false;
    }
    m_listenSocket = int(server);
    m_listenPort = port;
    QRLoginClient::TargetInfo ti(hi);
    m_pty = new QRLoginPtyClient(ti, this);
    if(!m_pty->init(m_listenSocket, m_listenPort)){
        return false;
    }
    QObject::connect(m_pty, SIGNAL(dataArrived(QByteArray)), this, SIGNAL(dataArrived(QByteArray)));
    QObject::connect(m_pty, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(m_pty, SIGNAL(errorArrived(QByteArray)), this, SIGNAL(errorArrived(QByteArray)));
    QObject::connect(m_pty, SIGNAL(passwordArrived(QString,QByteArray)), this, SIGNAL(passwordArrived(QString,QByteArray)));
    QObject::connect(m_pty, SIGNAL(inputArrived(QString,QString,bool)), this, SLOT(onInputArrived(QString,QString,bool)));
    return true;
}

void QWoRLogin::cleanup()
{
    if(m_listenSocket > 0) {
        myclosesocket(socket_t(m_listenSocket));
    }
    m_listenSocket = 0;
    m_listenPort = 0;
    if(m_pty) {
        m_pty->stop();
    }
    m_input = nullptr;
    m_pty = nullptr;
}

QWoRLoginFactory::QWoRLoginFactory(QObject *parent)
{

}

QWoRLoginFactory::~QWoRLoginFactory()
{

}

QWoRLoginFactory *QWoRLoginFactory::instance()
{
    static QPointer<QWoRLoginFactory> factory = new QWoRLoginFactory();
    return factory;
}

QWoRLogin *QWoRLoginFactory::create()
{
    return new QWoRLogin(this);
}

void QWoRLoginFactory::release(QWoRLogin *obj)
{
    obj->disconnect();
    QObject::connect(obj, SIGNAL(finishArrived(int)), this, SLOT(onFinished(int)));
    if(!obj->hasRunning()) {
        obj->deleteLater();
        return;
    }
    obj->stop();
    m_dels.append(obj);
}

void QWoRLoginFactory::onFinished(int)
{
    cleanup();
}

void QWoRLoginFactory::onAboutToQuit()
{

}

void QWoRLoginFactory::cleanup()
{
    for(QList<QPointer<QWoRLogin>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
        QWoRLogin *obj = *iter;
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
