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

#ifndef QKXVNC_H
#define QKXVNC_H

#include "qkxvnc_share.h"

#include <QThread>
#include <QPointer>
#include <QList>
#include <QImage>
#include <QMutex>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
struct VNCMsg;

class QKxAudioPlayer;
class QKXVNC_EXPORT QKxVNC : public QThread
{
    Q_OBJECT
public:
    enum EPixelFormat {
        Auto = 0,
        RGB32_888 = 1,
        RGB16_565 = 2,
        RGB15_555 = 3,
        RGB8_332 = 4,
        RGB8_Map = 5,
        YUV_NV12 = 6,
        H264_High = 7,
        H264_Normal = 8,
        H264_Low = 9,
        H264_Lowest = 10,
        JPEG_444 = 11
    };

    enum EProtoVersion {
        RFB_Invalid = 0,
        RFB_33 = 1,
        RFB_37 = 2,
        RFB_38 = 3
    };

    enum EMessageType {
        EMT_FameUpdate = 0,
        EMT_ColorMapEntry = 1,
        EMT_Bell = 2,
        EMT_CutText = 3,
        EMT_BlackScreen = 100,
        EMT_LockScreen = 101,
        EMT_Ftp = 102,
        EMT_PlayAudio = 103,
        EMT_PrivacyScreen = 104
    };

    enum EEncodingType
    {
        Raw = 0,
        CopyRect = 1,
        RRE = 2,
        Hextile = 5,
        TRLE = 15,
        ZRLE = 16,
        OpenH264 = 50,
        JPEG = 51,
        TRLE2 = 100,
        ZRLE2 = 101,
        TRLE3 = 102,
        ZRLE3 = 103,

        WoVNCScreenCount = -100,
        WoVNCMessageSupport = -101,
        CursorPseudoEncoding = -239,
        DesktopSizePseudoEncoding = -223,
        ClipboardEncoding = 0xc0a1e5ce,
    };
    struct TargetInfo {
        QByteArray host;
        ushort port;
        QByteArray password;
        EPixelFormat fmt;
        EProtoVersion proto;
        QVector<EEncodingType> vet;
    };
public:
    explicit QKxVNC(QObject *parent=nullptr);
    virtual ~QKxVNC();
    bool start(const QByteArray& host, int port, const QByteArray& passwd, EPixelFormat fmt = RGB16_565, EProtoVersion proto=RFB_38, QVector<EEncodingType> vet=QVector<EEncodingType>());
    void setInputResult(const QString& passwd);
    QString hostPort() const;
    void setPixelFormat(EPixelFormat fmt);
    EPixelFormat getPixelFormat();
    bool hasRunning();
    void stop();

    void setAudioEnabled(bool on);
    void setPrivacyScreenEnabled(bool on);

    QImage capture();
    QRect clip(const QSize& sz, const QRect& imgRt = QRect());
    int screenCount() const;
    bool isWoVNCServer() const;
    bool isAudioPlay() const;
    // idx== length; mean all show.
    QRect screenRect(int idx) const;
    bool supportAudio() const;
    bool supportFtp() const;
    bool supportBlackScreen() const;
    bool supportLockScreen() const;
    bool supportMessage(EMessageType emt) const;
    void setBlackScreen();
    void setLockScreen();
    bool sendFtpPacket(const QByteArray& data);
    int fps() const;
    void setFPS(int fps);
    // mouse event
    void handleMouseEvent(QMouseEvent *ev, const QSize& sz, const QRect& imgRt = QRect());
    void wheelEvent(QWheelEvent *ev, const QSize& sz);

    // key event
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);
    void sendClientCutText(const QString& buf);

    // update
    void updateRequest();
    void restoreKeyboardStatus();
    void setLockKeySymbol(bool on);
    bool lockKeySymbole();
signals:
    void qpsArrived(int qps);
    void ftpArrived(const QByteArray& data);
    void audioStateArrived(int state);
    void privacyStateArrived(int state);
    void pcmArrived(const QByteArray& pcm);
    void messageSupport(int cnt);
    void screenCountChanged(int cnt);
    void connectionStart();
    void connectionFinished(bool ok);
    void bellAlert();
    void cutTextArrived(const QString& buf);
    void updateArrived();
    void errorArrived(const QByteArray& buf);
    void passwordResult(const QByteArray& passwd, bool ok);
    void inputArrived(const QString& prompt, bool visible);

private:
signals:
    void finishArrived(int v);
    // internal
    void internalUpdateArrived(const QRect& rt);
private slots:
    void onFinished();
    void onInternalUpdateArrived(const QRect& rt);
protected:
    virtual void sendMouseEvent(int button, int x, int y);
    virtual void sendKeyEvent(quint32 key, bool down);
    virtual bool handleInput(const QString& prompt, QByteArray& result, bool visble);
protected:
    void push(uchar type, const QByteArray& data = QByteArray());
    bool pop(uchar &type, QByteArray& data);
    bool takeOne(uchar type, QByteArray& data);
    qint32 translateKey(QKeyEvent *ev);
protected:
    TargetInfo m_ti;
    QImage m_desktop;
    QList<QRect> m_screenRect;
    QList<qint32> m_typSupport;
    QRect m_clip;
    QMutex m_mutex;
    QList<VNCMsg> m_queue;
    int m_msgRead;
    int m_msgWrite;
    int m_fps;
    bool m_isFromWoVNCServer;

    QPointer<QKxAudioPlayer> m_player;
};

class QKxVNCFactory : public QObject
{
    Q_OBJECT
public:
    explicit QKxVNCFactory(QObject *parent=nullptr);
    ~QKxVNCFactory();
    static QKxVNCFactory *instance();
    QKxVNC *create();
    void release(QKxVNC *obj);
private slots:
    void onFinished(int);
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QKxVNC>> m_dels;
    QMutex m_mutex;
};

#endif // QKXVNC_H
