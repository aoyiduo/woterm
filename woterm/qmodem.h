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

#pragma once

#include <QThread>
#include <QPointer>
#include <QWaitCondition>
#include <QMutex>
#include <QTimer>

struct ZModem;
struct ZPrivate;
class QModem : public QThread
{
    Q_OBJECT
public:
    enum Protocol{
        PT_None,
        PT_XModem,
        PT_YModem,
        PT_ZModem
    };
    struct ZMsg{
        char type;
        QByteArray data;
    };

public:
    explicit QModem(bool isTelnet, QObject *parent=nullptr);
    ~QModem();

    bool ZSendFiles(const QStringList& files, Protocol protocol=PT_ZModem);
    bool ZReceive(const QString& path, Protocol protocol=PT_ZModem);
    bool onReceive(const QByteArray& buf);
    void stop();
public:
signals:
    void dataArrived(const QByteArray& buf);
    void statusArrived(const QByteArray& status);
private:
    int	ZXmitChr(uchar c) ;
    int	ZXmitStr(uchar *str, int len) ;
    void ZIFlush() ;
    void ZOFlush() ;
    int	ZAttn() ;
    void ZStatus(int type, int value, char *status);
    void ZFlowControl(int onoff);
    FILE *ZOpenFile(char *name, ulong crc) ;
    int ZWriteFile(uchar *buffer, int len, FILE *file);
    int ZCloseFile();
    void ZIdleStr(unsigned char *buf, int len);
private:
    static int _ZXmitChr(uchar c, QModem *that);
    static int	_ZXmitStr(uchar *str, int len, QModem *that);
    static void _ZIFlush(QModem *that);
    static void _ZOFlush(QModem *that);
    static int	_ZAttn(QModem *that);
    static void _ZStatus(int type, int value, char *status, QModem *that);
    static void _ZFlowControl(int onoff, QModem *that);
    static FILE *_ZOpenFile(char *name, ulong crc, QModem *that) ;
    static int _ZWriteFile(uchar *buffer, int len, FILE *file, QModem *that);
    static int _ZCloseFile(QModem *that);
    static void _ZIdleStr(unsigned char *buf, int len, QModem *that);
private:
    virtual void run();
    virtual int sending();
    virtual int receiving();
private:
    void push(char type, const QByteArray& data = QByteArray());
    bool pop(char &type, QByteArray& data);
private:
    int InitXmit();
    int XmitFile(const QString& file);
    int FinishXmit();
    int doIO();
    void formatError(int err, const QString& fileName);
    void formatStatus(int type, int value, char *msg);
    void status(const QString& msg, bool newLine=false);
    bool init();
    void exitLater();
    void cleanup();
private slots:
    void onTickerTimeout();
private:
    const bool m_bTelnet;
    bool m_bInit;
    bool m_bWaitForExit;
    bool m_bSend;
    Protocol m_protocol;
    ZModem *m_zmodem;
    ZPrivate *m_prv;
    QStringList m_files; // for send
    QString m_path; // for save
    bool m_stop;
    QWaitCondition m_sigal;
    QMutex m_mutex;
    QList<ZMsg> m_queue;
    QTimer m_ticker;
};

class QWoModemFactory : public QObject
{
    Q_OBJECT
public:
    explicit QWoModemFactory(QObject *parent=nullptr);
    ~QWoModemFactory();
    static QWoModemFactory *instance();
    QModem *create(bool isTelnet);
    void release(QModem *obj);
private slots:
    void onFinished();
    void onAboutToQuit();
private:
    void cleanup();
private:
    QList<QPointer<QModem>> m_dels;
    QMutex m_mutex;
};
