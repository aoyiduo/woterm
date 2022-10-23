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

#include "qkxvnc.h"
#include "qkxutils.h"
#include "qkxbuffer.h"
#include "qkxh264decoder.h"
#include "keysymdef.h"
#include "qkxaudioplayer.h"
#include "qkxopusdecoder.h"
#include "qkxjpegdecoder.h"

#include <libyuv.h>

#include <QMutexLocker>
#include <QDebug>
#include <QtEndian>
#include <QDateTime>
#include <QMouseEvent>
#include <QtMath>

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

#include "qkxzip.h"
#include "qkxsocket.h"
#include "d3des.h"

#define COMPUTE_TIME(x) \
{\
    qint64 t1 = QDateTime::currentMSecsSinceEpoch(); \
    x; \
    qint64 t2 = QDateTime::currentMSecsSinceEpoch(); \
    qDebug() << "Time Used:" << t2 - t1; \
}

#define MT_MOUSE            (1)
#define MT_KEY              (2)
#define MT_UPDATE           (3)
#define MT_PIXEL            (4)
#define MT_INPUT            (5)
#define MT_CUTTEXT          (6)
#define MT_BLACKSCREEN      (7)
#define MT_LOCKSCREEN       (8)
#define MT_FTP              (9)
#define MT_AUDIO            (10)
#define MT_PRIVACYSCREEN    (11)
#define MT_EXIT         (0x1F)
struct VNCMsg {
    uchar type;
    QByteArray data;
};

#ifdef Q_OS_WIN
bool init() {
    WORD sockVersion = MAKEWORD(2,2);
    WSADATA wsaData;
    int err = WSAStartup(sockVersion, &wsaData);
    return err == 0;
}
static bool __init = init();
#endif

QKxVNC:: QKxVNC(QObject *parent)
    : QThread(parent)
{
    m_msgRead = -1;
    m_msgWrite = -1;
    m_fps = 30;
    m_isFromWoVNCServer = false;
    QObject::connect(this, SIGNAL(internalUpdateArrived(QRect)), this, SLOT(onInternalUpdateArrived(QRect)));
    QObject::connect(this, SIGNAL(finished()), this, SLOT(onFinished()), Qt::QueuedConnection);

    m_player = new QKxAudioPlayer(48000, 2, this);
    QObject::connect(this, SIGNAL(pcmArrived(QByteArray)), m_player, SLOT(onAudioBuffer(QByteArray)));
}

QKxVNC::~QKxVNC()
{
    delete m_player;
    if(m_msgRead > 0) {
        myclosesocket(m_msgRead);
    }
    if(m_msgWrite > 0) {
        myclosesocket(m_msgWrite);
    }
}

bool QKxVNC::start(const QByteArray &host, int port, const QByteArray &passwd, EPixelFormat fmt, EProtoVersion proto, QVector<EEncodingType> vet)
{
    m_ti.host = host;
    m_ti.port = port;
    m_ti.password = passwd;
    m_ti.fmt = fmt;
    m_ti.proto = proto;
    m_ti.vet = vet;

    int fd[2] = {0};
    if(!QKxUtils::createPair2(10217, fd)) {
        return false;
    }
    m_msgRead = fd[0];
    m_msgWrite = fd[1];
    QKxUtils::setSocketNoDelay(fd[0], true);
    QKxUtils::setSocketNoDelay(fd[1], true);
    emit connectionStart();
    QThread::start();
    return true;
}

void QKxVNC::setInputResult(const QString &passwd)
{
    push(MT_INPUT, passwd.toUtf8());
}

QString QKxVNC::hostPort() const
{
    return QString(m_ti.host)+QString::number(m_ti.port);
}

void QKxVNC::setPixelFormat(QKxVNC::EPixelFormat fmt)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << (int)fmt;
    push(MT_PIXEL, buf);
}

QKxVNC::EPixelFormat QKxVNC::getPixelFormat()
{
    return m_ti.fmt;
}

bool QKxVNC::hasRunning()
{    
    return isRunning();
}

void QKxVNC::stop()
{
    for(int i = 0; i < 50; i++) {
        push(MT_EXIT);
    }
    qDebug() << "left" << left;
}

void QKxVNC::setAudioEnabled(bool on)
{
    if(m_typSupport.contains(EMT_PlayAudio)) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << on;
        push(MT_AUDIO, buf);

        if(on) {
            m_player->start();
        }else{
            m_player->stop();
        }
    }
}

void QKxVNC::setPrivacyScreenEnabled(bool on)
{
    if(m_typSupport.contains(EMT_PrivacyScreen)) {
        QByteArray buf;
        QDataStream ds(&buf, QIODevice::WriteOnly);
        ds << on;
        push(MT_PRIVACYSCREEN, buf);
    }
}

QImage QKxVNC::capture()
{
    return m_desktop;
}

QRect QKxVNC::clip(const QSize &sz, const QRect& imgRt)
{
    if(m_desktop.isNull()) {
        return QRect();
    }
    QRect clip = m_clip;
    if(imgRt.isEmpty()) {
        int width = m_desktop.width();
        int height = m_desktop.height();
        int x = m_clip.x() * sz.width() / width;
        int y = m_clip.y() * sz.height() / height;
        int w = m_clip.width() * sz.width() / width;
        int h = m_clip.height() * sz.height() / height;
        m_clip = QRect();
        return QRect(x-2, y-2, w+4, h+4);
    }else{
        clip &= imgRt;
        int width = imgRt.width();
        int height = imgRt.height();
        clip.translate(-imgRt.left(), -imgRt.top());
        int x = clip.x() * sz.width() / width;
        int y = clip.y() * sz.height() / height;
        int w = clip.width() * sz.width() / width;
        int h = clip.height() * sz.height() / height;
        m_clip = QRect();
        return QRect(x-2, y-2, w+4, h+4);
    }
}

int QKxVNC::screenCount() const
{
    return m_screenRect.length();
}

bool QKxVNC::isWoVNCServer() const
{
    return m_isFromWoVNCServer;
}

bool QKxVNC::isAudioPlay() const
{
    return m_player != nullptr;
}

QRect QKxVNC::screenRect(int idx) const
{
    if(m_screenRect.isEmpty()) {
        return QRect();
    }else if(idx >= m_screenRect.length()) {
        return m_desktop.rect();
    }else if(idx < 0) {
        return m_screenRect.at(0);
    }
    return m_screenRect.at(idx);
}

bool QKxVNC::supportAudio() const
{
    return m_typSupport.contains(EMT_PlayAudio);
}

bool QKxVNC::supportFtp() const
{
    return m_typSupport.contains(EMT_Ftp);
}

bool QKxVNC::supportBlackScreen() const
{
    return m_typSupport.contains(EMT_BlackScreen);
}

bool QKxVNC::supportLockScreen() const {
    return m_typSupport.contains(EMT_LockScreen);
}

bool QKxVNC::supportMessage(EMessageType emt) const
{
    return m_typSupport.contains(emt);
}

void QKxVNC::setBlackScreen()
{
    if(m_typSupport.contains(EMT_BlackScreen)) {
        push(MT_BLACKSCREEN);
    }
}

void QKxVNC::setLockScreen()
{
    if(m_typSupport.contains(EMT_BlackScreen)) {
        push(MT_LOCKSCREEN);
    }
}

bool QKxVNC::sendFtpPacket(const QByteArray &data)
{
    if(m_typSupport.contains(EMT_Ftp)) {
        push(MT_FTP, data);
    }
    return true;
}

int QKxVNC::fps() const
{
    return m_fps;
}

void QKxVNC::setFPS(int fps)
{
    m_fps = fps;
}

void QKxVNC::handleMouseEvent(QMouseEvent *ev, const QSize &sz, const QRect& imgRt)
{
    QPoint pt = ev->pos();
    if(m_desktop.isNull()) {
        return;
    }
    int x = 0;
    int y = 0;
    if(imgRt.isEmpty()) {
        int width = m_desktop.width();
        int height = m_desktop.height();
        //qDebug() << "mousePressEvent" << pt;
        x = pt.x() * width / sz.width();
        y = pt.y() * height / sz.height();
    }else{
        int width = imgRt.width();
        int height = imgRt.height();
        x = pt.x() * width / sz.width();
        y = pt.y() * height / sz.height();
        x += imgRt.left();
        y += imgRt.top();
    }

    Qt::MouseButtons mbs = ev->buttons();
    quint8 button = 0;
    if(mbs & Qt::LeftButton) {
        button |= 0x1;
    }
    if(mbs & Qt::MiddleButton) {
        button |= 0x2;
    }
    if(mbs & Qt::RightButton) {
        button |= 0x4;
    }
    sendMouseEvent(button, x, y);
}

void QKxVNC::wheelEvent(QWheelEvent *ev, const QSize &sz)
{
    QPoint pt = ev->pos();
    if(m_desktop.isNull()) {
        return;
    }
    int width = m_desktop.width();
    int height = m_desktop.height();
    //qDebug() << "wheelEvent" << pt;
    int x = pt.x() * width / sz.width();
    int y = pt.y() * height / sz.height();

    int delta = ev->delta();
    Qt::MouseButtons mbs = ev->buttons();
    quint8 button = 0;
    if(mbs & Qt::LeftButton) {
        button |= 0x1;
    }
    if(mbs & Qt::MiddleButton) {
        button |= 0x2;
    }
    if(mbs & Qt::RightButton) {
        button |= 0x4;
    }
    if(delta > 0) {
        button |= 0x8;
    }else{
        button |= 0x10;
    }
    sendMouseEvent(button, x, y);
    button &= ~0x18;
    sendMouseEvent(button, x, y);
}

void QKxVNC::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_CapsLock || ev->key() == Qt::Key_NumLock) {
       // Servers should ignore "lock" keysyms such as CapsLock and NumLock
       // where possible. Instead, they should interpret each character-
       // based keysym according to its case.
        return;
    }
    qint32 key = translateKey(ev);
    if(key < 0) {
        // don't send the key not match vnc protocol.
        return;
    }
    sendKeyEvent(key, true);
}

void QKxVNC::keyReleaseEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_CapsLock || ev->key() == Qt::Key_NumLock) {
        // Servers should ignore "lock" keysyms such as CapsLock and NumLock
        // where possible. Instead, they should interpret each character-
        // based keysym according to its case.
        return;
    }
    qint32 key = translateKey(ev);
    if(key < 0) {
        // don't send the key not match vnc protocol.
        return;
    }
    sendKeyEvent(key, false);
}

void QKxVNC::sendClientCutText(const QString &buf)
{
    push(MT_CUTTEXT, buf.toLatin1());
}

void QKxVNC::updateRequest()
{
    push(MT_UPDATE);
}

void QKxVNC::restoreKeyboardStatus()
{
//    sendKeyEvent(0xffe1, false);
//    sendKeyEvent(0xffe2, false);
//    sendKeyEvent(0xffe3, false);
//    sendKeyEvent(0xffe4, false);
//    sendKeyEvent(0xffe7, false);
//    sendKeyEvent(0xffe8, false);
//    sendKeyEvent(0xffe9, false);
//    sendKeyEvent(0xffea, false);
}

void QKxVNC::onFinished()
{
    emit finishArrived(0);
}

void QKxVNC::onInternalUpdateArrived(const QRect& rt)
{
    m_clip |= rt;
    emit updateArrived();
}

void QKxVNC::sendMouseEvent(int button, int x, int y)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << button << x << y;
    //qDebug() << button << x << y;
    push(MT_MOUSE, buf);
}

void QKxVNC::sendKeyEvent(quint32 key, bool down)
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << key << down;
    push(MT_KEY, buf);
}

bool QKxVNC::handleInput(const QString &prompt, QByteArray &result, bool visble)
{
    emit inputArrived(prompt, visble);
    fd_set fds;
    while(1) {
        uchar t;
        FD_ZERO(&fds);
        FD_SET(m_msgRead, &fds);
        int n = select(m_msgRead+1, &fds, nullptr, nullptr, nullptr);
        if(QKxUtils::xRecv(m_msgRead, (char*)&t, 1) <= 0) {
            return false;
        }
        if(takeOne(MT_EXIT, result)) {
            return false;
        }
        if(takeOne(MT_INPUT, result)) {
            return true;
        }
    }
    return false;
}

void QKxVNC::push(uchar type, const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    VNCMsg tmp;
    tmp.type = type;
    tmp.data = data;
    m_queue.append(tmp);
    //qDebug() <<  "push" << m_queue.length() << type << data;
    ::send(m_msgWrite, (char*)&type, 1, 0);
}

bool QKxVNC::pop(uchar &type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    if(m_queue.isEmpty()) {
        return false;
    }
    VNCMsg tmp = m_queue.takeFirst();
    type = tmp.type;
    data = tmp.data;
    //qDebug() <<  "pop" << m_queue.length() << type << data;
    return true;
}

bool QKxVNC::takeOne(uchar type, QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    for(int i = 0; i < m_queue.length(); i++) {
        VNCMsg &msg = m_queue[i];
        if(msg.type == type) {
            data.swap(msg.data);
            m_queue.removeAt(i);
            return true;
        }
    }
    return false;
}

qint32 QKxVNC::translateKey(QKeyEvent *ev)
{
    int scanCode = ev->nativeScanCode();
    if(ev->key() >= Qt::Key_F1 && ev->key() <= Qt::Key_F12) {
        quint32 key = ev->key() - Qt::Key_F1 + VNC_KEY_F1;
        return key;
    }
    if(ev->key() >= Qt::Key_Space && ev->key() <= Qt::Key_AsciiTilde) {
        QByteArray lat = ev->text().toLatin1();
        if(!lat.isEmpty() && QChar::isPrint(lat.at(0))) {
            qDebug() << "Latin1Char" << lat.at(0) << QString::number(lat.at(0)) << ev->key() << ev->modifiers();
            return lat.at(0);
        }
        qDebug() << "come here" << ev->key();
        return ev->key();
    }
    switch(ev->key()) {
    case Qt::Key_Backspace:
        return VNC_KEY_BackSpace; //XK_BackSpace
    case Qt::Key_Tab:
        return VNC_KEY_Tab; //XK_Tab
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VNC_KEY_ReturnOrEnter;
    case Qt::Key_Escape:
        return VNC_KEY_Escape;
    case Qt::Key_Insert:
        return VNC_KEY_Insert;
    case Qt::Key_Delete:
        return VNC_KEY_Delete;
    case Qt::Key_Home:
        return VNC_KEY_Home;
    case Qt::Key_End:
        return VNC_KEY_End;
    case Qt::Key_PageUp:
        return VNC_KEY_Page_Up;
    case Qt::Key_PageDown:
        return VNC_KEY_Page_Down;
    case Qt::Key_Left:
        return VNC_KEY_Left;
    case Qt::Key_Up:
        return VNC_KEY_Up;
    case Qt::Key_Right:
        return VNC_KEY_Right;
    case Qt::Key_Down:
        return VNC_KEY_Down;
    case Qt::Key_Shift:
#ifdef Q_OS_WIN
        if(scanCode == 0x2a) {
            return VNC_KEY_Shift_Left;
        }
        return VNC_KEY_Shift_Right;
#else
#ifdef Q_OS_LINUX
        if(scanCode == 0x32) {
            return VNC_KEY_Shift_Left;
        }
        return VNC_KEY_Shift_Right;
#else
        if(scanCode == 1) {
            return VNC_KEY_Shift_Left;
        }
        return VNC_KEY_Shift_Right;
#endif
#endif
    case Qt::Key_Control:
#ifdef Q_OS_WIN
        if(scanCode == 0x1d) {
            return VNC_KEY_Control_Left;
        }
        return VNC_KEY_Control_Right;
#else
#ifdef Q_OS_LINUX
        if(scanCode == 0x25) {
            return VNC_KEY_Control_Left;
        }
        return VNC_KEY_Control_Right;
#else
        if(scanCode == 1) {
            return VNC_KEY_Control_Left;
        }
        return VNC_KEY_Control_Right;
#endif
#endif
    case Qt::Key_Meta:
#ifdef Q_OS_WIN
        if(scanCode == 0x15b) {
            return VNC_KEY_Meta_Left;
        }
        return VNC_KEY_Meta_Right;
#else
#ifdef Q_OS_LINUX
        if(scanCode == 0x85) {
            return VNC_KEY_Meta_Left;
        }
        return VNC_KEY_Meta_Right;
#else
        if(scanCode == 1) {
            return VNC_KEY_Meta_Left;
        }
        return VNC_KEY_Meta_Right;
#endif
#endif
    case Qt::Key_Alt:
#ifdef Q_OS_WIN
        if(scanCode == 0x38) {
            return VNC_KEY_Alt_Left;
        }
        return VNC_KEY_Alt_Right;
#else
#ifdef Q_OS_LINUX
        if(scanCode == 0x40) {
            return VNC_KEY_Alt_Left;
        }
        return VNC_KEY_Alt_Right;
#else
        if(scanCode == 1) {
            return VNC_KEY_Alt_Left;
        }
        return VNC_KEY_Alt_Right;
#endif
#endif
    case Qt::Key_Super_L:
        return -1;
    case Qt::Key_Super_R:
        return -1;
    case Qt::Key_CapsLock:
        return -1;
    case Qt::Key_NumLock:
        return -1;
    }
    return ev->key();
}

class QPowerVNC : public QKxVNC
{
    #pragma pack(push, 1)
    struct CutTextRequest {
        quint8 type;
        quint8 reserved[3];
        quint32 length;
    };
    #pragma pack(pop)
    #pragma pack(push, 1)
    struct UpdateRequest {
        quint8 type;
        quint8 incr;
        quint16 x;
        quint16 y;
        quint16 w;
        quint16 h;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct UpdateRectHeader {
        quint16 x;
        quint16 y;
        quint16 w;
        quint16 h;
        qint32 et;
    };
    #pragma pack(pop)

    enum ESecurityType {
        ST_Invalid = 0,
        ST_None = 1,
        ST_VNCAuth = 2,
        ST_RA2 = 5,
        ST_RA2ne = 6,
        ST_Tight = 16,
        ST_Ultra = 17,
        ST_TLS = 18,
        ST_VeNCrypt = 19,
        ST_GTKVNCSASL = 20,
        ST_MD5Hash = 21,
        ST_ColinDeanXvp = 22
    };    

    #pragma pack(push, 1)
    struct PixelFormat {
        quint8 bitsPerPixel; // 8: map or rgb232, 12: 420
        quint8 depth;
        quint8 bigEndian;
        quint8 trueColor;
        quint16 redMax;
        quint16 greenMax;
        quint16 blueMax;
        quint8 redShift;
        quint8 greenShift;
        quint8 blueShift;
        quint8 extendFormat; // 0: default and compatible, 1: yuv, 2: h264, 3: jpeg
        quint8 quality;  //0:high, 1:normal, 2:low.
        quint8 reserved;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct FrameInfo {
        quint16 width;
        quint16 height;
        PixelFormat pixel;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct PixelFormatRequest{
        quint8 type;
        quint8 reserved[3];
        PixelFormat fmt;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct EncodingRequest{
        quint8 type;
        quint8 reserved;
        quint16 cnt;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct MouseRequest{
        quint8 type;
        quint8 button;
        quint16 x;
        quint16 y;
    };
    #pragma pack(pop)
    #pragma pack(push, 1)
    struct KeyRequest{
        quint8 type;
        quint8 down;
        quint8 reserved[2];
        quint32 key;
    };
    #pragma pack(pop)
private:
    EPixelFormat m_fmt;
    EProtoVersion m_verUsed;
    QKxTcpSocket m_vnc;
    QByteArray m_msgLast;
    qint64 m_tmFrameLast;
    int m_frameLeft;

    //info.
    FrameInfo m_frame;
    QByteArray m_name;
    quint8 *m_rgbptr;
    QVector<QRgb> m_clrTable;

    QVector<QPointer<QKxH264Decoder>> m_decoder;
    QPointer<QKxOpusDecoder> m_opus;

    //pixel format
    static PixelFormat m_rgb32_888;
    static PixelFormat m_jpeg_444;
    static PixelFormat m_rgb16_565;
    static PixelFormat m_rgb15_555;
    static PixelFormat m_rgb8_332;
    static PixelFormat m_rgb8_map;
    static PixelFormat m_yuv_nv12;
    static PixelFormat m_h264_high;
    static PixelFormat m_h264_normal;
    static PixelFormat m_h264_low;
    static PixelFormat m_h264_lowest;

    QByteArray m_zbuf;
    QKxZip m_zip;

    // update model
    bool m_updateImmediately;
    bool m_bexit;    
public:
    explicit QPowerVNC(QObject *parent=nullptr)
        : QKxVNC(parent)
    {
        m_verUsed = RFB_Invalid;
        m_zbuf.reserve(100 * 1024);
        m_updateImmediately = false;
        m_bexit = false;
        m_tmFrameLast = QDateTime::currentMSecsSinceEpoch();
        m_frameLeft = 0;
    }

    ~QPowerVNC() {
        qDebug() << "Exit now";
    }

protected:
    static bool decendingLessThan(const qint32 &s1, const qint32 &s2) {
        return s1 > s2;
    }
    virtual void run() {
        int err = 0;
        try{
            err = _run();
            qDebug() << "exit run" << err;
        }catch(...) {
            m_msgLast = "exception ...";
            emit errorArrived(m_msgLast);
            qDebug() << m_msgLast;
        }
        emit finishArrived(err);
    }

    int _run() {
        if(!m_vnc.connect(m_ti.host, m_ti.port)) {
            emit connectionFinished(false);
            return -100;
        }
        if(!handleProtocol()) {
            emit connectionFinished(false);
            return -101;
        }
        if(!handleSecurity()) {
            return -102;
        }
        emit connectionFinished(true);
        if(!handleInit(true)) {
            emit connectionFinished(true);
            return -103;
        }
        if(!m_isFromWoVNCServer) {
            m_ti.vet.removeOne(OpenH264);
            m_ti.vet.removeOne(ZRLE3);
            m_ti.vet.removeOne(TRLE3);
            m_ti.vet.removeOne(ZRLE2);
            m_ti.vet.removeOne(TRLE2);
            m_ti.vet.removeOne(JPEG);
            m_ti.vet.removeOne(WoVNCScreenCount);
            m_ti.vet.removeOne(WoVNCMessageSupport);
        }else{
            m_ti.vet.append(WoVNCScreenCount);
            m_ti.vet.append(WoVNCMessageSupport);
        }
        QList<qint32> encs;
        if(m_ti.vet.isEmpty()) {
            encs << Raw << OpenH264 << JPEG << ZRLE3 << TRLE3 << ZRLE2 << TRLE2 << ZRLE << TRLE << Hextile << RRE << CopyRect << DesktopSizePseudoEncoding;
        }else{
            for(int i = 0; i < m_ti.vet.length(); i++) {
                encs << m_ti.vet.at(i);
            }
        }
        std::sort(encs.begin(), encs.end(), decendingLessThan);
        //encs << ZRLE << Raw;
        setEncodings(encs);
        setPixelFormat(m_ti.fmt);
        requestCutTextToServer();
        requestFramebufferUpdate(false, 0, 0, m_frame.width, m_frame.height);

        int fd_vnc = m_vnc.handle();
        int fd_max = fd_vnc > m_msgRead ? fd_vnc : m_msgRead;        
        m_updateImmediately = false;
        QKxUtils::setSocketNonBlock(m_msgRead, true);
        qint32 qps = 0;
        qint64 tmLast = QDateTime::currentSecsSinceEpoch();
        while(1) {
            timeval tm={0, 15000};
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd_vnc, &rfds);
            FD_SET(m_msgRead, &rfds);
            int n = select(fd_max+1, &rfds, nullptr, nullptr, &tm);
            if(n < 0) {
                return -1000 + n;
            }
            qint64 now = QDateTime::currentSecsSinceEpoch();
            if(now != tmLast) {
                emit qpsArrived(qps);
                qps = 0;
                tmLast = now;
            }

            if(n == 0) {
                requestFramebufferUpdate(true, 0, 0, m_frame.width, m_frame.height);
                continue;
            }

            if(FD_ISSET(m_msgRead, &rfds)) {
                char type;
                int cnt;
                do {
                    cnt = QKxUtils::xRecv(m_msgRead, (char*)&type, 1);
                    if(cnt > 0) {
                        uchar type;
                        QByteArray data;
                        while(pop(type, data)) {
                            //qDebug() << "pop" << type << data;
                            if(type == MT_EXIT) {
                                qDebug() << "running exit now" << m_ti.host;
                                return 0;
                            }else if(type == MT_MOUSE) {
                                int button, x, y;
                                QDataStream buf(data);
                                buf >> button >> x >> y;
                                requestMouseUpdate(button, x, y);
                            }else if(type == MT_KEY) {
                                quint32 key;
                                bool down;
                                QDataStream buf(data);
                                buf >> key >> down;
                                requestKeyUpdate(key, down);
                            }else if(type == MT_PIXEL) {
                                int fmt;
                                QDataStream buf(data);
                                buf >> fmt;
                                setPixelFormat((EPixelFormat)fmt);
                            }else if(type == MT_CUTTEXT) {
                                requestCutTextToServer(data);
                            }else if(type == MT_BLACKSCREEN) {
                                requestBlackScreen();
                            }else if(type == MT_LOCKSCREEN) {
                                requestLockScreen();
                            }else if(type == MT_FTP) {
                                requestFtpPacket(data);
                            }else if(type == MT_AUDIO) {
                                bool on;
                                QDataStream buf(data);
                                buf >> on;
                                requestAudioEnabled(on);
                            }else if(type == MT_PRIVACYSCREEN) {
                                bool on;
                                QDataStream buf(data);
                                buf >> on;
                                requestPrivacyScreenEnabled(on);
                            }
                        }
                        requestFramebufferUpdate(true, 0, 0, m_frame.width, m_frame.height);
                    }
                }while(cnt > 0);
                if(cnt < 0) {
                    return -1;
                }
            }

            if(FD_ISSET(fd_vnc, &rfds)) {
                qps++;
                do{
                    QRect rt;
                    quint8 type = m_vnc.readUint8();
                    //qDebug() << "type" << type << QDateTime::currentMSecsSinceEpoch();
                    switch(type) {
                    case EMT_FameUpdate:
                        if(!handleFrameBufferUpdate(rt)){
                            return -104;
                        }
                        emit internalUpdateArrived(rt);
                        break;
                    case EMT_ColorMapEntry:
                        if(!handleColorMapEntries()) {
                            return -105;
                        }
                        break;
                    case EMT_Bell:
                        if(!handleBell()) {
                            return -106;
                        }
                        break;
                    case EMT_CutText:
                        if(!handleServerCutText()) {
                            return -107;
                        }
                        break;
                    case EMT_BlackScreen:
                        if(!handleBlackScreenResult()) {
                            return -108;
                        }
                        break;
                    case EMT_LockScreen:
                        if(!handleLockScreenResult()) {
                            return -109;
                        }
                        break;
                    case EMT_PlayAudio:
                        if(!handleAudioPacket()) {
                            return -110;
                        }
                        break;
                    case EMT_Ftp:
                        if(!handleFtpPacket()) {
                            return -111;
                        }
                        break;
                    case EMT_PrivacyScreen:
                        if(!handlePrivacyScreenResult()) {
                            return -112;
                        }
                        break;
                    }
                }while(!m_vnc.isEmpty());
            }
        }
    }
protected:
    virtual void sendMouseEvent(int button, int x, int y){
        if(!m_updateImmediately) {
            QKxVNC::sendMouseEvent(button, x, y);
            return;
        }
        requestMouseUpdate(button, x, y);
        requestFramebufferUpdate(true, 0, 0, m_frame.width, m_frame.height);
    }

    virtual void sendKeyEvent(quint32 key, bool down){
        if(!m_updateImmediately) {
            QKxVNC::sendKeyEvent(key, down);
            return;
        }
        requestKeyUpdate(key, down);
        requestFramebufferUpdate(true, 0, 0, m_frame.width, m_frame.height);
    }

private:
    bool handleProtocol() {
        char buf[13] = {0};
        if(!m_vnc.waitRead(buf, 12)) {
            return false;
        }
        EProtoVersion verSupport = RFB_Invalid;
        if(strcmp(buf, "RFB 003.003\n") == 0) {
            verSupport = RFB_33;
        }else if(strcmp(buf, "RFB 003.007\n") == 0) {
            verSupport = RFB_37;
        }else if(strcmp(buf, "RFB 003.008\n") == 0) {
            verSupport = RFB_38;
        }else {
            return false;
        }
        EProtoVersion verWanted = m_ti.proto;
        m_verUsed = qMin(verWanted, verSupport);
        if(m_verUsed == RFB_33) {
            strcpy(buf, "RFB 003.003\n");
        }else if(m_verUsed == RFB_37) {
            strcpy(buf, "RFB 003.007\n");
        }else if(m_verUsed == RFB_38) {
            strcpy(buf, "RFB 003.008\n");
        }
        m_ti.proto = m_verUsed;
        int n = m_vnc.waitWrite(buf, 12);
        return n > 0;
    }


    bool handlePassword() {
        QByteArray passwd = m_ti.password;
        if(passwd.isEmpty()){
#if 1
            emit passwordResult(passwd, false);
            return false;
#else
            if(!handleInput(tr("Password"), passwd, false)) {
                m_bexit = true;
                return false;
            }
#endif
        }
        uchar key[9] = {0};
        for (int i = 0; i < 8; i++) {
            if (i < passwd.length()) {
                key[i] = passwd[i];
            } else {
                key[i] = 0;
            }
        }
        rfbDesKey(key, EN0);
        uchar out[17] = {0};
        uchar *challenge = (uchar*)m_vnc.current(16);
        m_vnc.readSkip(16);
        for (int i = 0; i < 16; i += 8) {
            rfbDes(challenge + i, out + i);
        }
        if(!m_vnc.waitWrite((char*)out, 16)) {
            return false;
        }
        quint32 result = m_vnc.readUint32();
        emit passwordResult(passwd, result == 0);
        return result == 0;
    }

    bool handleSecurityV33() {
        qint32 type = m_vnc.readInt32();
        if(type == ST_Invalid) {
            m_msgLast = m_vnc.readArray();
            return false;
        } else if(type == ST_None) {
            return true;
        } else if(type == ST_VNCAuth) {
            // just password.
            return handlePassword();
        }
        return false;
    }

    bool handleTLSAuth() {
        quint8 cnt = m_vnc.readUint8();
        quint8 ts[100];
        if(!m_vnc.waitRead((char*)ts, cnt)) {
            return false;
        }
        for(int i = 0; i < cnt; i++) {
            quint8 t = ts[i];
            switch(t) {
            case ST_Invalid:
                break;
            case ST_None:
                if(!m_vnc.waitWrite((char*)&t, 1)) {
                    return false;
                }
                return true;
            case ST_VNCAuth:
                break;
            }
        }
        return true;
    }

    bool handleSecurityV37() {
        quint8 cnt = m_vnc.readUint8();
        if(cnt == 0) {
            return false;
        }
        quint8 ts[100];
        if(!m_vnc.waitRead((char*)ts, cnt)) {
            return false;
        }
        for(int i = 0; i < cnt; i++) {
            quint8 t = ts[i];
            if(t == ST_Invalid) {
                m_msgLast = m_vnc.readArray();
                return false;
            }else if(t == ST_None) {
                if(!m_vnc.waitWrite((char*)&t, 1)) {
                    return false;
                }
                return true;
            }else if(t == ST_VNCAuth) {
                if(!m_vnc.waitWrite((char*)&t, 1)) {
                    return false;
                }
                return handlePassword();
            }else if(t == ST_TLS) {
                return false;
//                if(!m_vnc.makeTLS()) {
//                    return false;
//                }
//                if(!m_vnc.waitWrite((char*)&t, 1)) {
//                    return false;
//                }
//                return handleTLSAuth();
            }
        }
        return false;
    }

    bool handleSecurityV38() {
        quint8 cnt = m_vnc.readUint8();
        if(cnt == 0) {
            return false;
        }
        quint8 ts[100];
        if(!m_vnc.waitRead((char*)ts, cnt)) {
            return false;
        }
        for(int i = 0; i < cnt; i++) {
            quint8 t = ts[i];
            if(t == ST_Invalid) {
                m_msgLast = m_vnc.readArray();
                return false;
            }else if(t == ST_None) {
                if(!m_vnc.waitWrite((char*)&t, 1)) {
                    return false;
                }
                if(m_vnc.readUint32() != 0) {
                    m_msgLast = m_vnc.readArray();
                    return false;
                }
                return true;
            }else if(t == ST_VNCAuth) {
                if(!m_vnc.waitWrite((char*)&t, 1)) {
                    return false;
                }
                if(!handlePassword()) {
                    if(m_bexit) {
                        return false;
                    }
                    m_msgLast = m_vnc.readArray();
                    return false;
                }
                return true;
            }
        }
        return false;
    }

    bool handleSecurity() {
        if(m_verUsed == RFB_33) {
            return handleSecurityV33();
        }else if(m_verUsed == RFB_37) {
            return handleSecurityV37();
        }else if(m_verUsed == RFB_38) {
            return handleSecurityV38();
        }
        return false;
    }

    bool handleInit(bool shared) {
        qint8 flag = shared ? 1 : 0;
        if(m_vnc.waitWrite((char*)&flag, 1)!= 1) {
            return false;
        }
        if(!m_vnc.waitRead((char*)&m_frame, sizeof(m_frame))) {
            return false;
        }
        m_frame.height = qFromBigEndian(m_frame.height);
        m_frame.width = qFromBigEndian(m_frame.width);
        m_frame.pixel.blueMax = qFromBigEndian(m_frame.pixel.blueMax);
        m_frame.pixel.greenMax = qFromBigEndian(m_frame.pixel.greenMax);
        m_frame.pixel.redMax = qFromBigEndian(m_frame.pixel.redMax);
        m_name = m_vnc.readArray();
        m_isFromWoVNCServer = m_name.startsWith("WoVNCServer:");
        m_desktop = QImage(m_frame.width, m_frame.height, QImage::Format_RGB32);
        m_rgbptr = (quint8*)m_desktop.bits();
        m_desktop.fill(Qt::black);

        m_screenRect.append(QRect(0, 0, m_frame.width, m_frame.height));
        emit screenCountChanged(1);
        return true;
    }

    bool setPixelFormat(EPixelFormat pf) {
        PixelFormatRequest req = {0};
        switch(pf) {
        case RGB32_888:
            req.fmt = m_rgb32_888;
            m_frame.pixel = m_rgb32_888;
            break;
        case JPEG_444:
            req.fmt = m_jpeg_444;
            m_frame.pixel = m_jpeg_444;
            break;
        case H264_High:
            req.fmt = m_h264_high;
            m_frame.pixel = m_h264_high;
            break;
        case H264_Normal:
            req.fmt = m_h264_normal;
            m_frame.pixel = m_h264_normal;
            break;
        case H264_Low:
            req.fmt = m_h264_low;
            m_frame.pixel = m_h264_low;
            break;
        case H264_Lowest:
            req.fmt = m_h264_lowest;
            m_frame.pixel = m_h264_lowest;
            break;
        case YUV_NV12:
            req.fmt = m_yuv_nv12;
            m_frame.pixel = m_yuv_nv12;
            break;
        case RGB16_565:
            req.fmt = m_rgb16_565;
            m_frame.pixel = m_rgb16_565;
            break;
        case RGB15_555:
            req.fmt = m_rgb15_555;
            m_frame.pixel = m_rgb15_555;
            break;
        case RGB8_332:
            req.fmt = m_rgb8_332;
            m_frame.pixel = m_rgb8_332;
            break;
        case RGB8_Map:
            req.fmt = m_rgb8_map;
            m_frame.pixel = m_rgb8_map;
            break;

        }
        m_fmt = pf;
        m_ti.fmt = pf;
        req.type = 0;
        req.fmt.blueMax = qToBigEndian(req.fmt.blueMax);
        req.fmt.greenMax = qToBigEndian(req.fmt.greenMax);
        req.fmt.redMax = qToBigEndian(req.fmt.redMax);
        return m_vnc.waitWrite((char*)&req, sizeof(req));
    }

    void handlePixelFormat(EPixelFormat pf) {
        if(m_fmt == pf) {
            return;
        }
        qInfo() << "handlePixelFormat collect error" << m_fmt << pf;
        switch(pf) {
        case RGB32_888:
            m_frame.pixel = m_rgb32_888;
            break;
        case JPEG_444:
            m_frame.pixel = m_jpeg_444;
            break;
        case RGB16_565:
            m_frame.pixel = m_rgb16_565;
            break;
        case RGB15_555:
            m_frame.pixel = m_rgb15_555;
            break;
        case RGB8_332:
            m_frame.pixel = m_rgb8_332;
            break;
        case RGB8_Map:
            m_frame.pixel = m_rgb8_map;
            break;
        case YUV_NV12:
            m_frame.pixel = m_yuv_nv12;
            break;
        case H264_High:
            m_frame.pixel = m_h264_high;
            break;
        case H264_Normal:
            m_frame.pixel = m_h264_normal;
            break;
        case H264_Low:
            m_frame.pixel = m_h264_low;
            break;
        case H264_Lowest:
            m_frame.pixel = m_h264_lowest;
            break;
        }
        m_fmt = pf;
        m_ti.fmt = pf;
    }

    bool setEncodings(QList<qint32> ecs) {
        QByteArray buf;
        buf.resize(sizeof(EncodingRequest) + ecs.length() * 4);
        EncodingRequest *req = (EncodingRequest*)buf.data();
        req->cnt = qToBigEndian<quint16>(ecs.length());
        req->type = 2;
        qint32 *pcodings = (qint32*)(buf.data() + sizeof(EncodingRequest));
        for(int i = 0; i < ecs.length(); i++) {
            pcodings[i] = qToBigEndian<quint32>(ecs.at(i));
        }
        return m_vnc.waitWrite(buf.data(), buf.length());
    }

    bool requestMouseUpdate(int button, int x, int y) {
        QMutexLocker locker(&m_mutex);
        MouseRequest req = {0};
        req.type = 5;
        req.button = button;
        req.x = qToBigEndian<quint16>(x);
        req.y = qToBigEndian<quint16>(y);
        return m_vnc.waitWrite((char*)&req, sizeof(req));
    }

    bool requestKeyUpdate(quint32 key, bool down) {
        QMutexLocker locker(&m_mutex);
        KeyRequest req = {0};
        req.type = 4;
        req.down = down;
        req.key = qToBigEndian(key);
        return m_vnc.waitWrite((char*)&req, sizeof(req));
    }

    bool requestFramebufferUpdate(bool incr, quint16 x, quint16 y, quint16 w, quint16 h) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        int interval = 1000 / m_fps;
        int elapse = now - m_tmFrameLast;
        int cnt = elapse * m_fps / 1000;
        m_frameLeft += cnt;
        m_tmFrameLast += cnt * 1000 / m_fps;
        if(m_frameLeft > (m_fps * 1.1)) {
            m_frameLeft = m_fps * 1.1;
            m_tmFrameLast = now;
        }else if(m_frameLeft <= 0) {
            m_frameLeft = 0;
            //qDebug() << "drop it" << now;
            return true;
        }
        m_frameLeft--;

        QMutexLocker locker(&m_mutex);
        UpdateRequest req;
        req.type = 3;
        req.incr = incr ? 1 : 0;
        req.x = qToBigEndian(x);
        req.y = qToBigEndian(y);
        req.w = qToBigEndian(w);
        req.h = qToBigEndian(h);
        return m_vnc.waitWrite((char*)&req, sizeof(req));
    }

    bool requestCutTextToServer(const QByteArray& buf = QByteArray()) {
        CutTextRequest req={0};
        req.type = 6;
        req.length = qToBigEndian<qint32>(buf.length());
        if(buf.isEmpty()) {
            return true;
        }
        if(!m_vnc.waitWrite((char*)&req, sizeof(req))) {
            return false;
        }
        if(buf.isEmpty()) {
            return true;
        }
        return m_vnc.waitWrite((char*)buf.data(), buf.length());
    }

    bool requestBlackScreen() {
        char cmd[2]{EMT_BlackScreen, 0};
        return m_vnc.waitWrite(cmd, 2);
    }

    bool requestLockScreen() {
        char cmd[2]{EMT_LockScreen, 0};
        return m_vnc.waitWrite(cmd, 2);
    }

    bool requestFtpPacket(const QByteArray& buf) {
        char cmd[5]{EMT_Ftp, 0};
        quint32 *pLength = reinterpret_cast<quint32*>(cmd+1);
        *pLength = qToBigEndian<quint32>(buf.length());
        m_vnc.waitWrite(cmd, 5);
        m_vnc.waitWrite(buf);
        return true;
    }

    bool requestAudioEnabled(bool on) {
        char enable = on ? char(1) : char(0);
        char cmd[3]{EMT_PlayAudio, char(1), enable};
        m_vnc.waitWrite(cmd, 3);
        return true;
    }

    bool requestPrivacyScreenEnabled(bool on) {
        char enable = on ? char(1) : char(0);
        char cmd[2]{EMT_PrivacyScreen, enable};
        m_vnc.waitWrite(cmd, 2);
        return true;
    }

    bool handleFrameBufferUpdate(QRect &rt) {
        qint8 fmt = m_vnc.readInt8();
        if(m_isFromWoVNCServer) {
            //qDebug() << "handleFrameBufferUpdate" << fmt;
            switch (fmt) {
            case 28:
                handlePixelFormat(JPEG_444);
                break;
            case 25:
                handlePixelFormat(RGB32_888);
                break;
            case 22:
                handlePixelFormat(H264_High);
                break;
            case 21:
                handlePixelFormat(H264_Normal);
                break;
            case 20:
                handlePixelFormat(H264_Low);
                break;
            case 19:
                handlePixelFormat(H264_Lowest);
                break;
            case 18:
                handlePixelFormat(YUV_NV12);
                break;
            case 17:
                handlePixelFormat(RGB16_565);
                break;
            case 16:
                handlePixelFormat(RGB15_555);
                break;
            case 9:
                handlePixelFormat(RGB8_332);
                break;
            case 8:
                handlePixelFormat(RGB8_Map);
                break;
            }
        }
        quint16 rtcnt = m_vnc.readUint16();
        for(int i = 0; i < rtcnt; i++) {
            quint16 x = m_vnc.readUint16();
            quint16 y = m_vnc.readUint16();
            quint16 w = m_vnc.readUint16();
            quint16 h = m_vnc.readUint16();
            qint32 et = m_vnc.readInt32();
            rt |= QRect(x, y, w, h);
            //qDebug() << "update" << et << x << y << w << h;
            switch(et) {
            case Raw:
                doFrameRaw(x, y, w, h);
                break;
            case CopyRect:
                doFrameCopyRect(x, y, w, h);
                break;
            case RRE:
                doFrameRRE(x, y, w, h);
                break;
            case Hextile:
                doFrameHextile(x, y, w, h);
                break;
            case TRLE:
            case TRLE2:
                doFrameTRLE(x, y, w, h);
                break;
            case ZRLE:
            case ZRLE2:
                doFrameZRLE(x, y, w, h);
                break;
            case TRLE3:
                doFrameTRLE3(x, y, w, h);
                break;
            case ZRLE3:
                doFrameZRLE3(x, y, w, h);
                break;
            case OpenH264:
                doFrameOpenH264(x, y, w, h);
                break;
            case JPEG:
                doFrameJpeg(x, y, w, h);
                break;
            case CursorPseudoEncoding:
                doCursorPseudoEncoding(x, y, w, h);
                break;
            case DesktopSizePseudoEncoding:
                doDesktopSizePseudoEncoding(x, y, w, h);
                break;
            case WoVNCScreenCount:
                m_isFromWoVNCServer = true;
                doScreenCount(x, y, w, h);
                break;
            case WoVNCMessageSupport:
                m_isFromWoVNCServer = true;
                doMessageSupport(x, y, w, h);
                break;
            }
        }        
        return true;
    }

    bool handleColorMapEntries() {
        quint8 padding;
        if(!m_vnc.waitRead((char*)&padding, 1)) {
            return false;
        }
        quint16 firstColor;
        if(!m_vnc.waitRead((char*)&firstColor, 2)) {
            return false;
        }
        firstColor = qFromBigEndian(firstColor);
        quint16 total;
        if(!m_vnc.waitRead((char*)&total, 2)) {
            return false;
        }
        total = qFromBigEndian(total);
        QVector<QRgb> clrTable;
        for(int i = 0; i < firstColor; i++) {
            clrTable.append(0);
        }
        QByteArray buf(total * 6, Qt::Uninitialized);
        if(!m_vnc.waitRead(buf.data(), buf.length())) {
            return false;
        }
        quint16 *pbuf = (quint16*)buf.data();
        for(int i = 0; i < total; i++) {
            int idx = i * 3;
            quint16 r = qFromBigEndian(pbuf[idx]);
            quint16 g = qFromBigEndian(pbuf[idx+1]);
            quint16 b = qFromBigEndian(pbuf[idx+2]);
            clrTable.append(qRgb(r >> 8, g >> 8, b >> 8));
        }
        m_clrTable = clrTable;
        return true;
    }

    bool handleBell() {
        emit bellAlert();
        return true;
    }

    bool handleServerCutText() {
        m_vnc.readSkip(3);
        QByteArray buf = m_vnc.readArray();
        emit cutTextArrived(QString::fromLatin1(buf));
        return true;
    }

    bool handleBlackScreenResult() {
        return true;
    }

    bool handleLockScreenResult() {
        return true;
    }

    bool handlePrivacyScreenResult() {
        qint8 on = m_vnc.readInt8();
        emit privacyStateArrived(on);
        return true;
    }

    bool handleAudioPacket() {
        qint8 t = m_vnc.readInt8();
        if(t == 1) {
            qint8 state = m_vnc.readInt8();
            emit audioStateArrived(state);
        }else if(t == 3) {
            QByteArray buf = m_vnc.readArray();
            // qDebug() << "opus data" << buf.length();
            if(m_opus == nullptr) {
                m_opus = new QKxOpusDecoder(48000, 2, this);
            }
            QByteArray pcm = m_opus->process(buf);
            if(!pcm.isEmpty()) {
                emit pcmArrived(pcm);
            }
        }
        return true;
    }

    bool handleFtpPacket() {
        qint32 cnt = m_vnc.readInt32();
        char *ptr = m_vnc.current(cnt);
        m_vnc.readSkip(cnt);
        QByteArray packet;
        packet.resize(cnt+4);
        qint32 *pint = reinterpret_cast<qint32*>(packet.data());
        *pint = qToBigEndian<qint32>(cnt);
        memcpy(packet.data() + 4, ptr, cnt);
        emit ftpArrived(packet);
        return true;
    }

    quint32 toRgbColor(quint32 rgb) {
        int r = ((rgb >> m_frame.pixel.redShift) & m_frame.pixel.redMax);
        r = qCeil((double)r / (double)m_frame.pixel.redMax * (double)255);
        int g = ((rgb >> m_frame.pixel.greenShift) & m_frame.pixel.greenMax);
        g = qCeil((double)g / (double)m_frame.pixel.greenMax * (double)255);
        int b = ((rgb >> m_frame.pixel.blueShift) & m_frame.pixel.blueMax);
        b = qCeil((double)b / (double)m_frame.pixel.blueMax * (double)255);
        return qRgb(r, g, b);
    }

    quint32 readPixelColor(QKxReader& in) {
        switch(m_fmt){
        case JPEG_444:
        case RGB32_888:
            return in.readRgb32() | 0xFF000000;
        case RGB16_565:
        case RGB15_555:
            return toRgbColor(in.readRgb16());
        case RGB8_332:
            return toRgbColor(in.readRgb8());
        case RGB8_Map:
            return m_clrTable.at(in.readRgb8());
        }
        return 0;
    }

    quint32 readCPixelColor(QKxReader &in) {
        if(m_fmt == RGB32_888 || m_fmt == JPEG_444) {
            return in.readRgb24() | 0xFF000000;
        }
        return readPixelColor(in);
    }

    bool doFrameRaw(quint16 x, quint16 y, quint16 width, quint16 height) {
        for(int h = 0; h < height; h++) {
            quint32 *line = myscanLine(y+h);
            quint32 *pdst = line + x;
            for(int w = 0; w < width; w++) {
                pdst[w] = readPixelColor(m_vnc);
            }
        }
        return true;
    }

    bool doFrameCopyRect(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint16 srcX = m_vnc.readUint16();
        quint16 srcY = m_vnc.readUint16();
        int nbytes = m_frame.pixel.bitsPerPixel / 8;
        if(y > srcY) {
            quint8* pdst = (quint8*)myscanLine(y+height-1);
            quint8 *psrc = (quint8*)myscanLine(srcY+height-1);
            pdst += x * nbytes;
            psrc += srcX * nbytes;
            for(qint16 h = height-1; h >= 0; h--) {
                memmove(pdst, psrc, width * nbytes);
                pdst -= m_desktop.bytesPerLine();
                psrc -= m_desktop.bytesPerLine();
            }
        }else{
            quint8* pdst = (quint8*)myscanLine(y);
            quint8 *psrc = (quint8*)myscanLine(srcY);
            pdst += x * nbytes;
            psrc += srcX * nbytes;
            for(quint16 h = 0; h < height; h++) {
                memmove(pdst, psrc, width * nbytes);
                pdst += m_desktop.bytesPerLine();
                psrc += m_desktop.bytesPerLine();
            }
        }

        return true;
    }

    bool doFrameRRE(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 cnt = m_vnc.readUint32();
        quint32 clr = readPixelColor(m_vnc);
        for(int h = y; h < y + height; h++) {
            quint32 *line = myscanLine(h);
            quint32 *pdst = line + x;
            for(int w = 0; w < width; w++) {
                pdst[w] = clr;
            }
        }
        for(int i = 0; i < cnt; i++) {
            quint32 clr = readPixelColor(m_vnc);
            quint16 subX = m_vnc.readUint16();
            quint16 subY = m_vnc.readUint16();
            quint16 subW = m_vnc.readUint16();
            quint16 subH = m_vnc.readUint16();
            for(int h = y + subY; h < y + subY + subH; h++) {
                quint32 *line = myscanLine(h);
                quint32 *pdst = line + x + subX;
                for(int w = 0; w < subW; w++) {
                    pdst[w] = clr;
                }
            }
        }
        return true;
    }

    void doPalleteRLE(QKxReader &buf, quint16 sub, quint16 x, quint16 y, quint16 width, quint16 height) {

    }

    bool doRgbRLE(QKxReader &buf, quint16 bsize, quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 rgbval = 0;
        quint8 btype = 0;
        qint32 runLength = 0;
        for(quint16 h = y; h < height + y; h += bsize) {
            quint16 bheight = qMin<quint16>(h + bsize, y + height) - h;
            for (quint16 w = x; w < width + x; w += bsize) {
                quint16 bwidth = qMin<quint16>(w + bsize, x + width) - w;
                bool bSame = false;
                for(quint16 bh = 0; bh < bheight; bh++) {
                    quint32 *line = myscanLine(h + bh);
                    quint32 *pdst = line + w;                    
                    for(quint16 bw = 0; bw < bwidth; bw++) {
                        if(runLength == 0) {
                            btype = buf.readUint8();
                            if(btype == 255) {
                                bSame = true;
                                break;
                            }else if(btype == 254) {
                                runLength = buf.readUint16();
                            }else if(btype == 253) {
                                runLength = buf.readUint32();
                            }else if(btype >= 168) {
                                runLength = btype - 168 + 1;
                            }else if(btype == 167) {
                                runLength = buf.readUint16();
                            }else if(btype == 166) {
                                runLength = buf.readUint32();
                            }else if(btype >= 85) {
                                runLength = btype - 85 + 1;
                            }else if(btype == 84) {
                                runLength = buf.readUint16();
                                rgbval = readCPixelColor(buf);
                            }else if(btype == 83) {
                                runLength = buf.readUint32();
                                rgbval = readCPixelColor(buf);
                            }else{
                                runLength = btype + 1;
                                rgbval = readCPixelColor(buf);
                            }
                        }
                        runLength--;
                        if(btype <= 84) {
                            pdst[bw] = rgbval;
                        }else if(btype >= 85 && btype <= 167) {
                            rgbval = readCPixelColor(buf);
                            pdst[bw] = rgbval;
                        }
                    }
                    if(bSame) {
                        break;
                    }
                }
           }
        }
        return true;
    }

    inline qint32 clamp0(qint32 v) {
      return -(v >= 0) & v;
    }

    inline qint32 clamp255(qint32 v) {
      return (-(v >= 255) | v) & 255;
    }

    quint32 Clamp(qint32 val) {
      int v = clamp0(val);
      return (quint32)(clamp255(v));
    }

    void YuvPixel(quint8 y, quint8 u, quint8 v, quint8* b, quint8* g, quint8* r) {
        //*b = *g = *r = 0;
        //return ;
        int ub = 128;
        int ug = 25;
        int vg = 52;
        int vr = 102;
        int yg = 18997;
        int yb = -1160;
        quint32 y32 = y * 0x0101;
        qint32 y1 = ((quint32)(y32 * yg) >> 16) + yb;
        qint8 ui = u;
        qint8 vi = v;
        ui -= 0x80;
        vi -= 0x80;
        int b16 = y1 + (ui * ub);
        int g16 = y1 - (ui * ug + vi * vg);
        int r16 = y1 + (vi * vr);
        *b = Clamp((qint32)(b16) >> 6);
        *g = Clamp((qint32)(g16) >> 6);
        *r = Clamp((qint32)(r16) >> 6);
    }

    void readYuvToRgb(QKxReader &buf, quint32 rgbval[4]) {
        quint32 rgb;
        quint8 *ptmp = (quint8*)&rgb;
        quint8 *pclr = (quint8*)buf.current(6);
        buf.readSkip(6);
        YuvPixel(pclr[0], pclr[4], pclr[5], ptmp, ptmp+1, ptmp+2);
        rgbval[0] = rgb;
        YuvPixel(pclr[1], pclr[4], pclr[5], ptmp, ptmp+1, ptmp+2);
        rgbval[1] = rgb;
        YuvPixel(pclr[2], pclr[4], pclr[5], ptmp, ptmp+1, ptmp+2);
        rgbval[2] = rgb;
        YuvPixel(pclr[3], pclr[4], pclr[5], ptmp, ptmp+1, ptmp+2);
        rgbval[3] = rgb;
    }

    bool doYuvRLE(QKxReader &buf, quint16 bsize, quint16 x, quint16 y, quint16 width, quint16 height) {
        return true;
    }

    bool doFrameTRLE3(quint16 x, quint16 y, quint16 width, quint16 height) {
        if(m_fmt == YUV_NV12) {
            return doYuvRLE(m_vnc, 16, x, y, width, height);
        }
        return doRgbRLE(m_vnc, 16, x, y, width, height);
    }

    bool doFrameZRLE3(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 zlen = m_vnc.readUint32();
        quint8* zbuf = (quint8*)m_vnc.current(zlen);
        m_vnc.readSkip(zlen);

        m_zbuf.clear();
        int total = m_zip.decode(QByteArray::fromRawData((char*)zbuf, zlen), m_zbuf);
        if(total < 0) {
            throw -1;
        }
        //qDebug() << "ZRLE total" << total;
        m_zbuf.resize(total);
        QKxBuffer buf(m_zbuf);

        return doRgbRLE(buf, 64, x, y, width, height);
    }

    QKxH264Decoder *findDecoder(quint16 x, quint16 y) {
        QKxH264Decoder *decoder = nullptr;
        if(m_isFromWoVNCServer) {
            for(int i = 0; i < m_screenRect.length(); i++) {
                const QRect &rt = m_screenRect.at(i);
                if(rt.contains(x, y)) {
                    if(m_decoder.length() <= i) {
                        decoder = new QKxH264Decoder(this);
                        decoder->init(rt.width(), rt.height());
                        m_decoder.append(decoder);
                    }else{
                        decoder = m_decoder[i];
                    }
                    break;
                }
            }
        }
        return decoder;
    }

    bool doFrameOpenH264(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 total = m_vnc.readUint32();
        quint32 flag = m_vnc.readUint32();
        quint8 *src = (quint8*)m_vnc.current(total);
        m_vnc.readSkip(total);
        QKxH264Decoder *decoder = findDecoder(x + width / 2, y + height / 2);
        if(decoder == nullptr) {
            // do nothing.
            return true;
        }
        uchar *yuv[3] = {0};
        int ystride, uvstride;
        int rv = decoder->decode(yuv, &ystride, &uvstride, src, total, flag == 0);
        if(rv == 1) {
            quint32 *pbuf = myscanLine(y);
            quint32 *pdst = pbuf + x;
            libyuv::I420ToARGB(yuv[0], ystride, yuv[1], uvstride, yuv[2], uvstride, (uchar*)pdst, m_desktop.bytesPerLine(), width, height);
        }
        //qDebug() << "decode" << rv;
        return true;
    }

    bool doFrameJpeg(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 total = m_vnc.readUint32();
        quint8 *src = (quint8*)m_vnc.current(total);
        m_vnc.readSkip(total);
        quint32 *rgb = myscanLine(y) + x;
        QKxJpegDecoder dec;
        int bytesPerLine = m_desktop.bytesPerLine();
        int length = bytesPerLine * height;
        dec.decode((uchar*)rgb, bytesPerLine, length, src, total);
        return true;
    }

    bool doCommonRLE(QKxReader &buf, quint16 bsize, quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 rgbval = 0;
        QVector<QRgb> clrTable;
        quint8 bitcnt = 0;
        for(quint16 h = y; h < height + y; h += bsize) {
            quint16 bheight = qMin<quint16>(h + bsize, y + height) - h;
            for (quint16 w = x; w < width + x; w += bsize) {
                quint16 bwidth = qMin<quint16>(w + bsize, x + width) - w;
                quint8 subcode = buf.readUint8();
                //qDebug() << "subcode" << subcode;
                if(subcode == 0) {
                    for(quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = line + w;
                        for(quint16 bw = 0; bw < bwidth; bw++) {
                            rgbval = readCPixelColor(buf);
                            pdst[bw] = rgbval;
                        }
                    }
                }else if(subcode == 1){
                    rgbval = readCPixelColor(buf);
                    for(quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = line + w;
                        for(quint16 bw = 0; bw < bwidth; bw++) {
                            pdst[bw] = rgbval;
                        }
                    }
                }else if(subcode >= 17 && subcode <= 126) {
                    // no used now.
                }else if (((subcode >= 2) && (subcode <= 16)) || (subcode == 127)) {
                    if (subcode != 127) {
                        clrTable.clear();
                        for (int i = 0; i < subcode; i++) {
                            clrTable.append(readCPixelColor(buf));
                        }
                        switch (subcode) {
                        case 2:
                            bitcnt = 1;
                            break;
                        case 3:
                        case 4:
                            bitcnt = 2;
                            break;
                        default:
                            bitcnt = 4;
                            break;
                        }
                    }

                    quint16 align = 0; /* align as 8 / 4 / 2 */
                    quint16 pixelcnt = 0; /* each line pixel count*/
                    quint8* pbuf = nullptr;
                    switch (bitcnt) {
                    case 1:
                        align = (quint16)floor((bwidth+7) / 8.0);
                        pbuf = (quint8*)buf.current(align * bheight);
                        buf.readSkip(align * bheight);
                        pixelcnt = align * 8;
                        break;
                    case 2:
                        align = (quint16)floor((bwidth+3) / 4.0);
                        pbuf = (quint8*)buf.current(align * bheight);
                        buf.readSkip(align * bheight);
                        pixelcnt = align * 4;
                        break;
                    case 4:
                    default:
                        align = (quint16)floor((bwidth+1) / 2.0);
                        pbuf = (quint8*)buf.current(align * bheight);
                        buf.readSkip(align * bheight);
                        pixelcnt = align * 2;
                        break;
                    }
                    quint16 i16 = 0;
                    quint16 i8 = 8 - bitcnt;
                    for (quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = line + w;
                        for (quint16 bw = 0; bw < pixelcnt; bw++) {
                            if (bw < bwidth) {
                                int i = (pbuf[i16] >> i8) & (0xFF >> (8 - bitcnt));
                                rgbval = clrTable.at(i);
                                pdst[bw] = rgbval;
                            }
                            if (i8 == 0) {
                                i8 = 8 - bitcnt;
                                i16++;
                            } else {
                                i8 -= bitcnt;
                            }
                        }
                    }
                }else if(subcode == 128) {
                    quint16 runLength = 0;
                    for(quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = line + w;
                        for(quint16 bw = 0; bw < bwidth; bw++) {
                            if(runLength == 0) {
                                rgbval = readCPixelColor(buf);
                                quint8 i8;
                                do{
                                    i8 = buf.readUint8();
                                    runLength += i8;
                                }while(i8 == 0xFF);
                                runLength++;
                            }
                            pdst[bw] = rgbval;
                            runLength--;
                        }
                    }
                }else if(subcode >= 129) {
                    if(subcode >= 130) {
                        clrTable.clear();
                        for(quint8 i = 128; i < subcode; i++) {
                            clrTable.append(readCPixelColor(buf));
                        }
                    }
                    quint16 runLength = 0;
                    for(quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = line + w;
                        for(quint16 bw = 0; bw < bwidth; bw++) {
                            if(runLength == 0) {
                                quint8 idx = buf.readUint8();
                                rgbval = clrTable.at(idx & 0x7F);
                                if(idx & 0x80) {
                                    quint8 i8;
                                    do{
                                        i8 = buf.readUint8();
                                        runLength += i8;
                                    }while(i8 == 0xFF);
                                }
                                runLength++;
                            }
                            pdst[bw] = rgbval;
                            runLength--;
                        }
                    }
                }
            }
        }
        return true;
    }

    bool doFrameHextile(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 bgclr, fgclr;
        bool bgok = false;
        bool fgok = false;
        for(quint16 h = y; h < y + height; h += 16) {
            quint16 bheight = qMin<quint16>(h + 16, y + height) - h;
            for(quint16 w = x; w < x + width;  w += 16) {
                quint16 bwidth = qMin<quint16>(w + 16, x + width) - w;
                quint8 encode = m_vnc.readUint8();
                bool raw = encode & 0x1;
                bool bgspec = encode & 0x2;
                bool fgspec = encode & 0x4;
                bool anyrt = encode & 0x8;
                bool rtclr = encode & 0x10;
                if(raw) {
                    for(quint16 bh = 0; bh < bheight; bh++) {
                        quint32 *line = myscanLine(h + bh);
                        quint32 *pdst = (quint32*)line + w;
                        for(quint16 bw = 0;  bw < bwidth; bw++) {
                            pdst[bw] = readPixelColor(m_vnc);
                        }
                    }
                }else{
                    if(bgspec) {
                        bgclr = readPixelColor(m_vnc);
                        bgok = true;
                    }
                    if(bgok){
                        for(quint16 bh = 0; bh < bheight; bh++) {
                            quint32 *line = myscanLine(h + bh);
                            quint32 *pdst = (quint32*)line + w;
                            for(quint16 bw = 0;  bw < bwidth; bw++) {
                                pdst[bw] = bgclr;
                            }
                        }
                    }
                    if(fgspec) {
                        fgclr = readPixelColor(m_vnc);
                        fgok = true;
                    }
                    if(anyrt) {
                        quint8 cnt = m_vnc.readUint8();
                        if(rtclr){
                            do{
                                cnt--;
                                quint32 clr = readPixelColor(m_vnc);
                                quint8 subX = m_vnc.readUint8();
                                quint8 subY = subX & 0xF;
                                subX = subX >> 4;
                                quint8 subW = m_vnc.readUint8();
                                quint8 subH = (subW & 0xF) + 1;
                                subW = (subW >> 4) + 1;
                                for(quint16 bh = subY; bh < subY+subH; bh++) {
                                    quint32 *line = myscanLine(h + bh);
                                    quint32 *pdst = (quint32*)line + w;
                                    for(quint16 bw = subX;  bw < subX+subW; bw++) {
                                        pdst[bw] = clr;
                                    }
                                }
                            }while(cnt > 0);
                        }else if(!fgok){
                            do{
                                cnt--;
                                m_vnc.readSkip(2);
                            }while(cnt > 0);
                        }else{
                            do{
                                cnt--;
                                quint32 clr = fgclr;
                                quint8 subX = m_vnc.readUint8();
                                quint8 subY = subX & 0xF;
                                subX = subX >> 4;
                                quint8 subW = m_vnc.readUint8();
                                quint8 subH = (subW & 0xF) + 1;
                                subW = (subW >> 4) + 1;
                                for(quint16 bh = subY; bh < subY+subH; bh++) {
                                    quint32 *line = myscanLine(h + bh);
                                    quint32 *pdst = (quint32*)line + w;
                                    for(quint16 bw = subX;  bw < subX+subW; bw++) {
                                        pdst[bw] = clr;
                                    }
                                }
                            }while(cnt > 0);
                        }
                    }
                }
            }
        }
        return true;
    }

    bool doFrameTRLE(quint16 x, quint16 y, quint16 width, quint16 height) {
        return doCommonRLE(m_vnc, 16, x, y, width, height);
    }

    bool doFrameZRLE(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint32 zlen = m_vnc.readUint32();
        quint8* zbuf = (quint8*)m_vnc.current(zlen);
        m_vnc.readSkip(zlen);

        m_zbuf.clear();
        int total = m_zip.decode(QByteArray::fromRawData((char*)zbuf, zlen), m_zbuf);
        if(total < 0) {
            throw -1;
        }
        //qDebug() << "ZRLE total" << total;
        m_zbuf.resize(total);
        QKxBuffer buf(m_zbuf);
        return doCommonRLE(buf, 64, x, y, width, height);
    }

    bool doCursorPseudoEncoding(quint16 x, quint16 y, quint16 width, quint16 height) {
        return false;
    }

    bool doDesktopSizePseudoEncoding(quint16 x, quint16 y, quint16 width, quint16 height) {
        m_frame.width = width;
        m_frame.height = height;
        m_desktop = QImage(m_frame.width, m_frame.height, QImage::Format_RGB32);
        m_rgbptr = (quint8*)m_desktop.bits();
        requestFramebufferUpdate(false, 0, 0, width, height);
        return true;
    }

    /***
     *   why don't use inner scanLine() function ?
     * because it's dangerous to use inner same name function.
     * on mult-thread, the scanline maybe will realloc the image data for some reason
     * so myself replace it.
     *
     ***/
    quint32 *myscanLine(int h) {
        return reinterpret_cast<quint32*>(m_rgbptr + h * m_desktop.bytesPerLine());
    }

    bool doScreenCount(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint8 cnt = m_vnc.readUint8();
        QList<QRect> rts;
        for(int i = 0; i < cnt; i++) {
            quint16 x = m_vnc.readUint16();
            quint16 y = m_vnc.readUint16();
            quint16 w = m_vnc.readUint16();
            quint16 h = m_vnc.readUint16();
            rts.append(QRect(x, y, w, h));
        }
        m_screenRect.swap(rts);
        emit screenCountChanged(cnt);
        return true;
    }

    bool doMessageSupport(quint16 x, quint16 y, quint16 width, quint16 height) {
        quint8 cnt = m_vnc.readUint8();
        QList<qint32> typs;
        for(int i = 0; i < cnt; i++) {
            qint32 typ = m_vnc.readInt32();
            typs.append(typ);
        }
        m_typSupport.swap(typs);
        emit messageSupport(cnt);
        return true;
    }
};

// Currently bits-per-pixel
// must be 8, 16, or 32
QPowerVNC::PixelFormat QPowerVNC::m_rgb32_888 = {32, 24, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0xFF, 0xFF, 0xFF, 16, 8, 0, 0, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_jpeg_444 = {24, 24, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0xFF, 0xFF, 0xFF, 16, 8, 0, 3, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_rgb16_565 = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 0, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_rgb15_555 = {16, 15, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x1F, 0x1F, 10, 5, 0, 0, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_rgb8_332 = {8, 8, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x07, 0x07, 0x03, 5, 2, 0, 0, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_rgb8_map = {8, 8, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_yuv_nv12 = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 1, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_h264_lowest = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 2, 0, 0};
QPowerVNC::PixelFormat QPowerVNC::m_h264_low = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 2, 1, 0};
QPowerVNC::PixelFormat QPowerVNC::m_h264_normal = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 2, 2, 0};
QPowerVNC::PixelFormat QPowerVNC::m_h264_high = {16, 16, Q_BYTE_ORDER == Q_BIG_ENDIAN ? 1 : 0, 1, 0x1F, 0x3F, 0x1F, 11, 5, 0, 2, 3, 0};

QKxVNCFactory::QKxVNCFactory(QObject *parent)
    : QObject(parent)
{

}

QKxVNCFactory::~QKxVNCFactory()
{

}

QKxVNCFactory *QKxVNCFactory::instance()
{
    static QPointer<QKxVNCFactory> factory = new QKxVNCFactory();
    return factory;
}

QKxVNC *QKxVNCFactory::create()
{
    return new QPowerVNC(this);
}

void QKxVNCFactory::release(QKxVNC *obj)
{
    obj->disconnect();
    bool ok = QObject::connect(obj, SIGNAL(finishArrived(int)), this, SLOT(onFinished(int)));
    if(!obj->hasRunning()) {
        obj->deleteLater();
        return;
    }
    obj->stop();
    m_dels.append(obj);
}

void QKxVNCFactory::onFinished(int)
{
    cleanup();
}

void QKxVNCFactory::onAboutToQuit()
{

}

void QKxVNCFactory::cleanup()
{
    for(QList<QPointer<QKxVNC>>::iterator iter = m_dels.begin(); iter != m_dels.end(); ) {
        QKxVNC *obj = *iter;
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
