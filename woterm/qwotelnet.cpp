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

#include "qwotelnet.h"
#include <libtelnet.h>

#include "qwoutils.h"
#include "qwosshconf.h"

#include <QMutexLocker>
#include <QDebug>
#include <QDataStream>
#include <QFile>

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
struct TelnetMsg {
    uchar type;
    QByteArray data;
};

static const telnet_telopt_t telopts[] = {
    { TELNET_TELOPT_ECHO,               TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_TTYPE,              TELNET_WILL, TELNET_DONT },
    { TELNET_TELOPT_COMPRESS2,          TELNET_WONT, TELNET_DONT },
    { TELNET_TELOPT_MSSP,               TELNET_WONT, TELNET_DO   },
    { TELNET_TELOPT_NAWS,               TELNET_WILL, TELNET_DO  },
    { -1, 0, 0 }
};

QTelnetClient::QTelnetClient(const TargetInfo& ti, QObject *parent)
    : QThread(parent)
    , m_ti(ti)
{
    m_cacheCount = 0;
    QObject::connect(this, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(this, SIGNAL(internalDataArrived(QByteArray)), this, SLOT(onInternalDataArrived(QByteArray)));
}

QTelnetClient::~QTelnetClient()
{
    onFinished();
}


bool QTelnetClient::init(int server, ushort port)
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

void QTelnetClient::stop()
{
    for(int i = 0; i < 50; i++) {
        push(MT_EXIT);
    }
    qDebug() << "left" << left;
}

#define MAX_CACHE   (30)
void QTelnetClient::emitDataArrived(const QByteArray &buf)
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

void QTelnetClient::onFinished()
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

void QTelnetClient::onInternalDataArrived(const QByteArray &buf)
{
    m_cacheCount--;
    emit dataArrived(buf);
}

void QTelnetClient::handleRead(const QByteArray &data)
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

void QTelnetClient::handleError(const QString &err)
{
    emit errorArrived(err.toUtf8());
}

void QTelnetClient::run()
{
    m_codeExit = running();
    qDebug() << "run exit now" << m_ti.name << m_ti.host;
}

int QTelnetClient::running()
{
    return -1;
}

void QTelnetClient::push(uchar type, const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    TelnetMsg tmp;
    tmp.type = type;
    tmp.data = data;
    m_queue.append(tmp);
    //qDebug() <<  "push" << m_queue.length() << type << data;
    ::send(m_msgWrite, (char*)&type, 1, 0);
}

bool QTelnetClient::pop(uchar &type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    if(m_queue.isEmpty()) {
        return false;
    }
    TelnetMsg tmp = m_queue.takeFirst();
    type = tmp.type;
    data = tmp.data;
    //qDebug() <<  "pop" << m_queue.length() << type << data;
    return true;
}

bool QTelnetClient::takeOne(uchar type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    for(int i = 0; i < m_queue.length(); i++) {
        TelnetMsg &msg = m_queue[i];
        if(msg.type == type) {
            data.swap(msg.data);
            m_queue.removeAt(i);
            return true;
        }
    }
    return false;
}

class QTelnetPtyClient : public QTelnetClient
{
private:
    socket_t m_fd;
    telnet_t *m_telnet;
    int m_flagEcho;
    int m_flagExit;
    int m_colsLast;
    int m_rowsLast;
public:
    QTelnetPtyClient(const TargetInfo &ti, QObject *parent)
        : QTelnetClient(ti, parent) {
        m_fd = -1;
        m_flagEcho = 1;
        m_flagExit = 0;
        m_telnet = nullptr;
    }
    ~QTelnetPtyClient() {
        cleanup();
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
        if(m_telnet) {
            telnet_free(m_telnet);
            m_telnet = nullptr;
        }
    }

    void telnet_winsize(int cols, int rows) {
        char cmd[4];
        cmd[0] = (char)(short(cols) >> 8);
        cmd[1] = (char)(short(cols) & 0xff);
        cmd[2] = (char)(short(rows) >> 8);
        cmd[3] = (char)(short(rows) & 0xff);
        telnet_subnegotiation(m_telnet, TELNET_TELOPT_NAWS, cmd, 4);
    }
    int ctrl(int x) {
        // 0x40: '@'
        // ctrl+A~Z, [0x41~0x5f]
        return '@' + x;
    }
    void telnet_special(int c) {
        switch(ctrl(c)){
        case 'C':
            telnet_iac(m_telnet, TELNET_IP);
            break;
        default:
            telnet_send(m_telnet, (char*)&c, 1);
         }
    }

    void telnet_txbinary() {
        uchar cmd[4] = {TELNET_WILL, 0, 0, 0};
        telnet_subnegotiation(m_telnet, TELNET_TELOPT_NAWS, (char*)cmd, 1);
    }

    void eventHandler(telnet_event_t *ev) {
        //qDebug() << "eventHandler" << ev->type;
        switch (ev->type) {
        /* data received */
        case TELNET_EV_DATA:
            if(ev->data.size > 0) {
                handleRead(QByteArray(ev->data.buffer, ev->data.size));
            }
            break;
            /* data must be sent */
        case TELNET_EV_SEND:
            QWoUtils::xSend(m_fd, (char*)ev->data.buffer, ev->data.size);
#if 0
             {
                QFile f("send.log");
                if(f.open(QFile::Append)) {
                    f.write(ev->data.buffer, ev->data.size);
                    f.write("\r\n---------------------------\r\n");
                    f.close();
                }
            }
#endif
            break;
            /* request to enable remote feature (or receipt) */
        case TELNET_EV_WILL:
            /* we'll agree to turn off our echo if server wants us to stop */
            qDebug() << "neg.telopt WILL" << ev->neg.telopt;
            if (ev->neg.telopt == TELNET_TELOPT_ECHO){
                m_flagEcho = 0;
            }
            break;
            /* notification of disabling remote feature (or receipt) */
        case TELNET_EV_WONT:
            qDebug() << "neg.telopt WONT" << ev->neg.telopt;
            if (ev->neg.telopt == TELNET_TELOPT_ECHO){
                m_flagEcho = 1;
            }
            break;
            /* request to enable local feature (or receipt) */
        case TELNET_EV_DO:
            qDebug() << "neg.telopt DO" << ev->neg.telopt;
            if(ev->neg.telopt == TELNET_TELOPT_NAWS) {
                telnet_winsize(m_colsLast, m_rowsLast);
            }else if(ev->neg.telopt == TELNET_TELOPT_BINARY) {
                telnet_txbinary();
            }
            break;
            /* demand to disable local feature (or receipt) */
        case TELNET_EV_DONT:
            qDebug() << "neg.telopt-DONT" << ev->neg.telopt;
            break;
            /* respond to TTYPE commands */
        case TELNET_EV_TTYPE:
            /* respond with our terminal type, if requested */
            if (ev->ttype.cmd == TELNET_TTYPE_SEND) {
                telnet_ttype_is(m_telnet, "xterm");
                //telnet_ttype_is(m_telnet, "vt102");
            }
            break;
            /* respond to particular subnegotiations */
        case TELNET_EV_SUBNEGOTIATION:
            qDebug() << "subnegotiation";
            break;
            /* error */
        case TELNET_EV_ERROR:
            handleError(QByteArray("ERROR: %s") + ev->error.msg);
            m_flagExit = 1;
        default:
            /* ignore */
            break;
        }
    }

    static void _event_handler(telnet_t *telnet, telnet_event_t *ev, void *user_data) {
        QTelnetPtyClient * pty = (QTelnetPtyClient*)user_data;
        pty->eventHandler(ev);
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

        for (itr = ai; itr != nullptr; itr = itr->ai_next) {
            /* create socket */
            socket_t fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);
            err = ::connect(fd, itr->ai_addr, itr->ai_addrlen);
            if(err != 0){
                myclosesocket(fd);
                continue;
            }
            m_fd = fd;
            break;
        }
        QWoUtils::freeAddrInfos(ai);
        if(m_fd < 0) {
            return -2;
        }
        m_telnet = telnet_init(telopts, _event_handler, 0, this);
        fd_set rfds;
        QWoUtils::setSocketNoDelay(m_msgRead, true);
        QWoUtils::setSocketNonBlock(m_msgRead, true);
        QWoUtils::setSocketNoDelay(m_fd, true);
        QWoUtils::setSocketNonBlock(m_fd, true);
        m_flagExit = 0;
        while(!m_flagExit) {
            timeval tm={3,0};
            FD_ZERO(&rfds);
            FD_SET(m_msgRead, &rfds);
            FD_SET(m_fd, &rfds);
            int fdmax = int(m_fd);
            if(fdmax < m_msgRead) {
                fdmax = m_msgRead;
            }
            int n = select(fdmax+1, &rfds, nullptr, nullptr, &tm);
            if(n == 0) {
                continue;
            }
            if(n < 0) {
                return -3;
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
                            telnet_send(m_telnet, data.data(), data.length());
                        }else if(type == MT_PTYSIZE) {
                            int cols, rows;
                            QDataStream buf(data);
                            buf >> cols >> rows;
                            telnet_winsize(cols, rows);
                        }else if(type == MT_CTRL) {
                            int c;
                            QDataStream buf(data);
                            buf >> c;
                            telnet_special(c);
                        }
                    }
                }else{
                    return -1;
                }
            }else if(FD_ISSET(m_fd, &rfds)) {
                QByteArray buf(10240, Qt::Uninitialized);
                int cnt = QWoUtils::xRecv(m_fd, buf.data(), buf.capacity());
                if( cnt > 0){
                    telnet_recv(m_telnet, buf.data(), cnt);
                }else{
                    return -1;
                }
            }
        }
        return 0;
    }
};

QWoTelnet::QWoTelnet(QObject *parent)
    : QObject(parent)
{

}

QWoTelnet::~QWoTelnet()
{
    cleanup();
}

bool QWoTelnet::hasRunning()
{
    if(m_pty == nullptr) {
        return false;
    }
    return m_pty->isRunning();
}


bool QWoTelnet::start(const QString &host)
{
    if(m_pty && m_pty->isRunning()) {
        return false;
    }
    if(!init(host)) {
        return false;
    }
    m_pty->start();
    return true;
}

void QWoTelnet::stop()
{
    if(m_pty) {
        m_pty->stop();
    }
}

void QWoTelnet::write(const QByteArray &buf)
{
    if(m_pty) {
        m_pty->write(buf);
    }
}

void QWoTelnet::updateSize(int cols, int rows)
{
    if(m_pty) {
        m_pty->updateSize(cols, rows);
    }
}

void QWoTelnet::sendControl(char c)
{
    m_pty->sendControl(c);
}

void QWoTelnet::onFinished()
{
    QTelnetClient *cli = qobject_cast<QTelnetClient*>(sender());
    if(cli) {
        int code = cli->exitCode();
        emit finishArrived(code);
    }
}

bool QWoTelnet::init(const QString &host)
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
    QTelnetClient::TargetInfo ti(hi);
    m_pty = new QTelnetPtyClient(ti, this);
    if(!m_pty->init(m_listenSocket, m_listenPort)){
        return false;
    }
    QObject::connect(m_pty, SIGNAL(dataArrived(const QByteArray&)), this, SIGNAL(dataArrived(const QByteArray&)));
    QObject::connect(m_pty, SIGNAL(finished()), this, SLOT(onFinished()));
    QObject::connect(m_pty, SIGNAL(errorArrived(const QByteArray&)), this, SIGNAL(errorArrived(const QByteArray&)));
    return true;
}

void QWoTelnet::cleanup()
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

QWoTelnetFactory::QWoTelnetFactory(QObject *parent)
{

}

QWoTelnetFactory::~QWoTelnetFactory()
{

}

QWoTelnetFactory *QWoTelnetFactory::instance()
{
    static QPointer<QWoTelnetFactory> factory = new QWoTelnetFactory();
    return factory;
}

QWoTelnet *QWoTelnetFactory::create()
{
    return new QWoTelnet(this);
}

void QWoTelnetFactory::release(QWoTelnet *obj)
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

void QWoTelnetFactory::onFinished(int)
{
    cleanup();
}

void QWoTelnetFactory::onAboutToQuit()
{

}

void QWoTelnetFactory::cleanup()
{
    for(QList<QPointer<QWoTelnet>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
        QWoTelnet *obj = *iter;
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
