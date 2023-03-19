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

#include "qwomodem.h"

#include "qwoutils.h"

#include <QDebug>
#include <zmodem.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTime>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>

#define myclosesocket  closesocket

#ifndef socket_t
typedef SOCKET socket_t;
#endif

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
#define myclosesocket(x) close(x)

#ifndef socket_t
typedef int socket_t;
#endif

#endif

typedef int (*FunZXmitChr)(uchar c, QWoModem *that);
typedef int	(*FunZXmitStr)(uchar *str, int len, QWoModem *that);
typedef void (*FunZIFlush)(QWoModem *that);
typedef void (*FunZOFlush)(QWoModem *that);
typedef int	(*FunZAttn)(QWoModem *that);
typedef void (*FunZStatus)(int type, int value, char *status, QWoModem *that);
typedef void (*FunZFlowControl)(int onoff, QWoModem *that);
typedef FILE *(*FunZOpenFile)(char *name, ulong crc, QWoModem *that) ;
typedef int (*FunZWriteFile)(uchar *buffer, int len, FILE *file, QWoModem *that);
typedef int (*FunZCloseFile)(QWoModem *that);
typedef void (*FunZIdleStr)(unsigned char *buf, int len, QWoModem *that);

struct ZPrivate
{
    socket_t rfd;
    socket_t wfd;
    QWoModem *that;
    FunZXmitChr ZXmitChr;
    FunZXmitStr ZXmitStr;
    FunZIFlush ZIFlush;
    FunZOFlush ZOFlush;
    FunZAttn ZAttn;
    FunZStatus ZStatus;
    FunZFlowControl ZFlowControl;
    FunZOpenFile ZOpenFile;
    FunZWriteFile ZWriteFile;
    FunZCloseFile ZCloseFile;
    FunZIdleStr ZIdleStr;
};

int	ZXmitChr(uchar c, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZXmitChr(c, p->that);
}

int	ZXmitStr(uchar *str, int len, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZXmitStr(str, len, p->that);
}

void ZIFlush(ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    p->ZIFlush(p->that);
}

void ZOFlush(ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    p->ZOFlush(p->that);
}

int	ZAttn(ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZAttn(p->that);
}

void ZFlowControl(int onoff, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    p->ZFlowControl(onoff, p->that);
}

void ZStatus(int type, int value, char *msg, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    p->ZStatus(type, value, msg, p->that);
}

FILE *ZOpenFile(char *name, ulong crc, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZOpenFile(name, crc, p->that);
}

int ZWriteFile( uchar *buffer, int len, FILE *file, ZModem *info )
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZWriteFile(buffer, len, file, p->that);
}

int ZCloseFile(ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZCloseFile(p->that);
}

void ZIdleStr(unsigned char *buf, int len, ZModem *info)
{
    ZPrivate *p = (ZPrivate*)(info + 1);
    return p->ZIdleStr(buf, len, p->that);
}

static void clearsocket(socket_t sock) {
    char buf[100];
    while(QWoUtils::xRecv(sock, buf, 99) > 0);
}

#define MT_DATA (1)
#define MT_EXIT (0x7f)

QWoModem::QWoModem(bool isTelnet, QObject *parent)
    : QThread(parent)
    , m_bTelnet(isTelnet)
{
    int length = sizeof(ZModem) + sizeof(ZPrivate);
    void *p = malloc(length);
    memset(p, 0, length);
    m_zmodem = (ZModem*)p;
    ZPrivate *prv = (ZPrivate*)(m_zmodem+1);
    prv->ZXmitChr = _ZXmitChr;
    prv->ZXmitStr = _ZXmitStr;
    prv->ZIFlush = _ZIFlush;
    prv->ZOFlush = _ZOFlush;
    prv->ZAttn = _ZAttn;
    prv->ZStatus = _ZStatus;
    prv->ZFlowControl = _ZFlowControl;
    prv->ZOpenFile = _ZOpenFile;
    prv->ZWriteFile = _ZWriteFile;
    prv->ZCloseFile = _ZCloseFile;
    prv->ZIdleStr = _ZIdleStr;
    prv->that = this;
    m_prv = prv;
    m_bInit = init();

    QObject::connect(&m_ticker, SIGNAL(timeout()), this, SLOT(onTickerTimeout()));
}

QWoModem::~QWoModem()
{
    free(m_zmodem);
    myclosesocket(m_prv->rfd);
    myclosesocket(m_prv->wfd);
}

bool QWoModem::init()
{
    int fd[2];
    if(!QWoUtils::createPair2(20327, fd)) {
        return false;
    }
    QWoUtils::setSocketNonBlock(fd[0], true);
    QWoUtils::setSocketNonBlock(fd[1], true);
    QWoUtils::setSocketNoDelay(fd[0], true);
    QWoUtils::setSocketNoDelay(fd[1], true);
    m_prv->rfd = fd[0];
    m_prv->wfd = fd[1];
    return true;
}

void QWoModem::exitLater()
{
    m_ticker.stop();
    m_ticker.setSingleShot(true);
    m_ticker.start(500);
}

bool QWoModem::ZSendFiles(const QStringList &files, Protocol protocol)
{
    if(!m_bInit) {
        return false;
    }
    if(isRunning()) {
       return false;
    }
    m_bWaitForExit = false;
    m_bSend = true;
    m_protocol = protocol;
    m_files = files;
    clearsocket(m_prv->rfd);
    m_queue.clear();
    QThread::start();
    return true;
}

bool QWoModem::ZReceive(const QString &path, Protocol protocol)
{
    if(!m_bInit) {
        return false;
    }
    if(isRunning()) {
       return false;
    }
    m_bWaitForExit = false;
    m_bSend = false;
    m_protocol = protocol;
    m_path = path;
    clearsocket(m_prv->rfd);
    m_queue.clear();
    QThread::start();
    return true;
}

bool QWoModem::onReceive(const QByteArray &buf)
{
    if(!isRunning()) {
        return false;
    }
    if(m_bWaitForExit) {
        exitLater();
        return false;
    }
    push(MT_DATA, buf);
    return true;
}

void QWoModem::stop()
{
    m_stop = true;
    push(MT_EXIT);
    qDebug() << "left" << left;
}

int QWoModem::ZXmitChr(uchar c)
{
    return 0;
}

int QWoModem::ZXmitStr(uchar *str, int len)
{
    QByteArray buf((const char*)str, len);
    emit dataArrived(buf);
    return 0;
}

void QWoModem::ZIFlush()
{

}

void QWoModem::ZOFlush()
{

}

int QWoModem::ZAttn()
{
    if ( m_zmodem->attn == nullptr ) {
        return 0;
    }

    int cnt = 0;
    for( char *ptr = m_zmodem->attn; *ptr != '\0'; ++ptr ) {
        if ( ( ( cnt++ ) % 10 ) == 0 ) {
            sleep(1);
        }
        if ( *ptr == ATTNBRK ) {
            //SerialBreak();
        } else if ( *ptr == ATTNPSE ) {
#if defined(_DEBUG)
            zmodemlog( "ATTNPSE\r\n" );
#endif
            sleep(1);
        }
    }
    return 0 ;
}

void QWoModem::ZStatus(int type, int value, char *msg)
{
    formatStatus(type, value, msg);
}

void QWoModem::ZFlowControl(int onoff)
{

}

FILE *QWoModem::ZOpenFile(char *name, ulong crc)
{
    if( *name == '/' ) {	/* for now, disallow absolute paths */
        return nullptr ;
    }
    QString path = QDir::toNativeSeparators(QDir::cleanPath(m_path + "/" + name));
    if(QFile::exists(path)) {
        return nullptr;
    }
    return fopen(path.toLocal8Bit(), "wb");
}

int QWoModem::ZWriteFile(uchar *buffer, int len, FILE *file)
{
    return fwrite(buffer, 1, len, file) == len ? 0 : ZmErrSys;
}

int QWoModem::ZCloseFile()
{
    fclose(m_zmodem->file);
    m_zmodem->file = nullptr;
    return 0;
}

void QWoModem::ZIdleStr(unsigned char *buf, int len)
{

}

int QWoModem::_ZXmitChr(uchar c, QWoModem *that)
{
    return that->ZXmitChr(c);
}

int QWoModem::_ZXmitStr(uchar *str, int len, QWoModem *that)
{
    return that->ZXmitStr(str, len);
}

void QWoModem::_ZIFlush(QWoModem *that)
{
    that->ZIFlush();
}

void QWoModem::_ZOFlush(QWoModem *that)
{
    that->ZOFlush();
}

int QWoModem::_ZAttn(QWoModem *that)
{
    return that->ZAttn();
}

void QWoModem::_ZStatus(int type, int value, char *status, QWoModem *that)
{
    that->ZStatus(type, value, status);
}

void QWoModem::_ZFlowControl(int onoff, QWoModem *that)
{
    that->ZFlowControl(onoff);
}

FILE *QWoModem::_ZOpenFile(char *name, ulong crc, QWoModem *that)
{
    return that->ZOpenFile(name, crc);
}

int QWoModem::_ZWriteFile(uchar *buffer, int len, FILE *file, QWoModem *that)
{
    return that->ZWriteFile(buffer, len, file);
}

int QWoModem::_ZCloseFile(QWoModem *that)
{
    return that->ZCloseFile();
}

void QWoModem::_ZIdleStr(unsigned char *buf, int len, QWoModem *that)
{
    that->ZIdleStr(buf, len);
}

void QWoModem::run()
{
    m_bWaitForExit = false;
    m_stop = false;
    emit status("\033[?25l");
    if(m_bSend) {
        sending();
    }else {
        receiving();
    }
    QMutexLocker lock(&m_mutex);
    m_bWaitForExit = true;
    // make sure had the last input string can reach onReceive
    emit dataArrived("\r\n");
    m_sigal.wait(&m_mutex, 60 * 1000);
    m_bWaitForExit = false;
    cleanup();
}

int QWoModem::sending()
{
    memset(m_zmodem, 0, sizeof(ZModem));
    m_zmodem->packetsize = 1024 ;
    m_zmodem->windowsize = 4096 ;
    m_zmodem->ifd = m_zmodem->ofd = -1;
    m_zmodem->telnet = m_bTelnet ? 1 : 0;
    //m_zmodem->zsinitflags = TESCCTL; // will cause transfer slowly.
    int done = InitXmit();
    if(done != ZmDone) {
        formatError(done, "");
    }else {
        for(int i = 0; i < m_files.count(); i++) {
            done = XmitFile(m_files.at(i));
            if(done != ZmDone) {
                break;
            }
        }
    }
    FinishXmit();
    return ZmDone;
}

int QWoModem::receiving()
{
    memset(m_zmodem, 0, sizeof(ZModem));
    m_zmodem->packetsize = 1024 ;
    m_zmodem->windowsize = 4096 ;
    m_zmodem->bufsize = 0;
    m_zmodem->telnet = m_bTelnet ? 1 : 0;
    int done = 0;
    if(m_protocol == PT_XModem || m_protocol == PT_YModem) {
        done = YmodemRInit(m_zmodem);
    }else{
        done = ZmodemRInit(m_zmodem);
    }
    if(!done) {
        done = doIO();
    }
    return done;
}

void QWoModem::status(const QString &msg, bool newLine)
{
    QByteArray buf;
    buf.append("\033[?25l\033[2K\033[0G");
    buf.append(msg.toUtf8());
    if(newLine) {
        buf.append("\r\n");
    }
    emit statusArrived(buf);
}

int QWoModem::doIO()
{    
    fd_set rfds;
    int done = 0;
    QByteArray buf(1024*10, Qt::Uninitialized);   
    while(!done) {
        timeval tm={10,0};
        FD_ZERO(&rfds);
        FD_SET(m_prv->rfd, &rfds);
        tm.tv_sec = m_zmodem->timeout;
        int n = select(m_prv->rfd+1, &rfds, nullptr, nullptr, &tm);
        if(n < 0) {
            return ZmErrInt;
        }else if(n == 0) {
            done = ZmodemTimeout(m_zmodem);
            if(done != 0) {
                return done;
            }
        }else{
            char type;
            if(QWoUtils::xRecv(m_prv->rfd, (char*)&type, 1) > 0) {
                char type;
                QByteArray data;
                while(pop(type, data)) {
                    if(type == MT_EXIT) {
                        qDebug() << "runing: recevie exit" << QTime::currentTime();
                        ZmodemAbort(m_zmodem);
                    } else if(type == MT_DATA) {
#if 0
                        QFile f("raw.log");
                        f.open(QFile::Append);
                        f.write(data);
                        f.write("\r\n--------------------------------\r\n");
                        f.close();
#endif
                        done = ZmodemRcv((uchar*)data.data(), data.length(), m_zmodem);
                        //qDebug() << "MT_DATA" << done;
                        if(done != 0) {
                            return done;
                        }

                    }
                }
            }
        }
    }
    return done;
}

void QWoModem::formatError(int err, const QString &fileName)
{
    switch (err) {
    case ZmDone:
        if(m_zmodem->offset == ulong(m_zmodem->len) && m_zmodem->fileEof) {
            status(QString("File transfer finish: %1").arg(fileName), true);
        }else{
            status(QString("Skipping %1").arg(fileName), true);
        }
        break;
    case ZmErrInt:
        status(QString("Failed to %1 for internal error.").arg(fileName));
        break;
    case ZmErrSys:
        status(QString("Failed to %1 for system error.").arg(fileName));
        break;
    case ZmErrNotOpen:
        status(QString("Failed to setup connection[%1].").arg(fileName));
        break;
    case ZmErrCantOpen:
        status(QString("Failed to open %1.").arg(fileName));
        break;
    case ZmFileTooLong:
        status(QString("Failed to %1 for name too long.").arg(fileName));
        break;
    case ZmFileCantWrite:
        status(QString("Failed to write %1").arg(fileName));
        break;
    case ZmDataErr:
        status(QString("Failed to %1 for two many data error").arg(fileName));
        break;
    case ZmErrInitTo:
        status(QString("Failed to initialize %1").arg(fileName));
        break;
    case ZmErrSequence:
        status(QString("Failed to %1 for bad data sequence.").arg(fileName));
        break;
    case ZmErrCancel:
        status(QString("Cancel for %1 by remote end").arg(fileName));
        break;
    case ZmErrRcvTo:
        status(QString("Time out to receive %1").arg(fileName));
        break;
    case ZmErrSndTo:
        status(QString("Time out send %1").arg(fileName));
        break;
    case ZmErrCmdTo:
        status(QString("Time out receive %1").arg(fileName));
        break;
    }
}

void QWoModem::formatStatus(int type, int value, char *msg)
{
    char tmp[] = "noName";
    char *fileName = tmp;
    if(m_zmodem->filename) {
        char *tmp = strrchr(m_zmodem->filename, '/');
        char *tmp2 = strrchr(m_zmodem->filename, '\\');
        if(tmp == nullptr && tmp2 == nullptr) {
            fileName = m_zmodem->filename;
        }else if(tmp == nullptr) {
            fileName = tmp2+1;
        }else if(tmp2 == nullptr) {
            fileName = tmp+1;
        }else if(tmp2 < tmp) {
            fileName = tmp + 1;
        }else {
            fileName = tmp2 + 1;
        }
    }
    switch( type ) {
    case RcvByteCount:
    {
        qint64 tmp = (m_zmodem->len + 1);
        qint64 v = qint64(value) * 100 / tmp;
        status(QString("%1: %2[%3][%4%] bytes received").arg(fileName).arg(value).arg(m_zmodem->len).arg(v));
        break;
    }
    case SndByteCount:
    {
        qint64 tmp = (m_zmodem->len + 1);
        qint64 v = qint64(value) * 100 / tmp;
        status(QString("%1: %2[%3][%4%] bytes send").arg(fileName).arg(value).arg(m_zmodem->len).arg(v));
        break;
    }
    case RcvTimeout:
        status("Receiver did not respond, aborting");
        break;

    case SndTimeout:
        status(QString("%1 send timeouts").arg(value));
        break;

    case RmtCancel:
        status(QString("Remote end has cancelled"));
        break;

    case ProtocolErr:
        status(QString("Protocol error has occurred, header: %1").arg(value));
        break;

    case RemoteMessage:	/* message from remote end */
        status(QString("Message from remote end: %1").arg(msg));
        break;

    case DataErr:		/* data error, val=error count */
    {
        status(QString("Data error, error count: %1").arg(value));
#if 0
        if(m_zmodem->state == TStart) {
            QFile f("err.txt");
            if(f.open(QFile::Append)) {
                f.write((char*)m_zmodem->buffer, m_zmodem->chrCount);
                f.write("\r\n----------------------\r\n");
                f.close();
            }
        }
#endif
        break;
    }

    case FileErr:		/* error writing file, val=errno */
        status(QString("Error writing file, val: %1").arg(strerror(errno)));
        break;

    case FileBegin:	/* file transfer begins, str=name */
        status(QString("File transfer begin: %1").arg(fileName));
        break;

    case FileEnd:		/* file transfer ends, str=name */
        status(QString("File transfer finish: %1").arg(fileName), true);
        break;

    case FileSkip:	/* file transfer ends, str=name */
        status(QString("Skipping %1").arg(fileName), true);
        break;
    }
}

void QWoModem::cleanup()
{
    if(m_zmodem->file) {
        fclose(m_zmodem->file);
        if(!m_bSend) {
            if(m_zmodem->filename){
                QByteArray path = QDir::toNativeSeparators(QDir::cleanPath(m_path + "/" + m_zmodem->filename)).toLocal8Bit();
                qDebug() << path;
                QFile::remove(path);
            }
        }
        m_zmodem->file = nullptr;
    }
    if(m_zmodem->filename) {
        free(m_zmodem->filename);
        m_zmodem->filename = nullptr;
    }
    if(m_zmodem->rfilename) {
        free(m_zmodem->rfilename);
        m_zmodem->rfilename = nullptr;
    }
    if(m_zmodem->buffer) {
        free(m_zmodem->buffer);
        m_zmodem->buffer = nullptr;
    }
    if(m_zmodem->attn) {
        free(m_zmodem->attn);
        m_zmodem->attn = nullptr;
    }
    m_protocol = PT_None;
    m_files.clear();
    m_path.clear();
    m_queue.clear();
    clearsocket(m_prv->rfd);
    m_bWaitForExit = false;
    qDebug() << "clean: zmodem exit" << QTime::currentTime();
}

void QWoModem::onTickerTimeout()
{
    m_ticker.stop();
    m_sigal.wakeAll();
}

void QWoModem::push(char type, const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    ZMsg msg;
    msg.type = type;
    msg.data = data;
    m_queue.push_back(msg);
    //qDebug() << "push" << m_queue.length();
    ::send(m_prv->wfd, (char*)&type, 1, 0);
}

bool QWoModem::pop(char &type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    if(m_queue.isEmpty()) {
        return false;
    }
    ZMsg tmp = m_queue.takeFirst();
    type = tmp.type;
    data = tmp.data;
    //qDebug() << "pop" << m_queue.length();
    return true;
}

int QWoModem::InitXmit()
{
    int done = 0;
    if(m_protocol == PT_XModem){
        done = XmodemTInit(m_zmodem);
    }else if(m_protocol == PT_YModem) {
        done = YmodemTInit(m_zmodem);
    }else {
        done = ZmodemTInit(m_zmodem);
    }
    if(!done) {
        done = doIO();
    }
    return done;
}

int QWoModem::XmitFile(const QString &file)
{
    int	done ;
    QFileInfo fi(file);
    // path : for local file open.
    QByteArray path = fi.absoluteFilePath().toLocal8Bit();
    // name : has to send to remote, so utf8 chars
    QByteArray name = fi.fileName().toUtf8();    
    done = ZmodemTFile(path, name, 0, 0, 0, 0, 0, 0, m_zmodem);
    if(done == 0){
        done = doIO();
    }
    formatError(done, name);
    return done;
}

int QWoModem::FinishXmit()
{
    int	done ;

    done = ZmodemTFinish(m_zmodem);
    if( !done )
        done = doIO();
    return done;
}

QWoModemFactory::QWoModemFactory(QObject *parent)
    : QObject(parent)
{

}

QWoModemFactory::~QWoModemFactory()
{

}

QWoModemFactory *QWoModemFactory::instance()
{
    static QPointer<QWoModemFactory> factory = new QWoModemFactory();
    return factory;
}

QWoModem *QWoModemFactory::create(bool isTelnet)
{
    return new QWoModem(isTelnet, this);
}

void QWoModemFactory::release(QWoModem *obj)
{
    obj->disconnect();
    QObject::connect(obj, SIGNAL(finished()), this, SLOT(onFinished()));
    if(!obj->isRunning()) {
        obj->deleteLater();
        return;
    }
    obj->stop();
    m_dels.append(obj);
}

void QWoModemFactory::onFinished()
{
    cleanup();
}

void QWoModemFactory::onAboutToQuit()
{

}

void QWoModemFactory::cleanup()
{
    for(QList<QPointer<QWoModem>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
        QWoModem *obj = *iter;
        if(obj == nullptr) {
            iter = m_dels.erase(iter);
            continue;
        }
        if(!obj->isRunning()) {
            obj->deleteLater();
            iter = m_dels.erase(iter);
            continue;
        }
        obj->stop();
        iter++;
    }
}
