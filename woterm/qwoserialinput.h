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

#ifndef QWOSERIALINPUT_H
#define QWOSERIALINPUT_H

#include <QWidget>
#include <QPointer>
#include <QList>
#include <QMap>

namespace Ui {
class QWoSerialInput;
}

class QPushButton;
class QTcpSocket;
class QTcpServer;
class QUdpSocket;
class QIODevice;
class QSerialPort;
class QAbstractSocket;
class QWoTermWidget;
class QTimer;
class QTextCodec;
class QWoSerialInput : public QWidget
{
    Q_OBJECT
private:
    struct TimeOutput {
        QByteArray buf;
        qint64 tmLast;
        TimeOutput() {
            tmLast = 0;
        }
    };

    struct LineOutput {
        QByteArray who;
        QByteArrayList lines;
    };
public:
    explicit QWoSerialInput(QWoTermWidget *term, QWidget *parent = 0);
    ~QWoSerialInput();
    void reset();
    bool isConnected();
signals:
    void moreReady();
private slots:
    void onSerialPortRefresh();
    void onServerTcpNewConnection();
    void onServerTcpReadyRead();
    void onServerTcpCleanup();
    void onClientTcpReadyRead();
    void onClientTcpCleanup();
    void onClientTcpCurrentTextChanged(const QString& name);
    void onServerUdpReadyRead();
    void onClientUdpReadyRead();
    void onServerUdpCleanup();
    void onComxReadyRead();
    void onDeviceBytesWritten(qint64 bytes);
    void onComxError();

    void onOutputTimeout();
    void onTermDataSend(const QByteArray& data);

    void onTcpListenButtonClicked();
    void onTcpConnectButtonClicked();
    void onUdpListenButtonClicked();
    void onUdpStartButtonClicked();
    void onComxConnectButtonClicked();
    void onTextSendButtonClicked();
    void onButtonSendFileClicked();
    void onSerialPortCurrentTextChanged(const QString& portName);
    void onPlainEditTextChanged();

    void onModeInputIndexChanged(int idx);
    void onModeSplitIndexChanged(int idx);
    void onModeOutputIndexChanged(int idx);

    void onSimulateSendButtonClicked();
private:
    bool handleTcpListen(bool start);
    void refleshTcpClients();
    bool handleUdpListen(bool start);
    bool handleTcpConnect(bool start);
    bool handleUdpConnect(bool start);
    bool handleComxConnect(bool start);

    QString socketName(const QAbstractSocket* socket) const;

    void handleDataRecv(const QString& who, const QByteArray& buf);    
    void handleDataSend(const QString& who, const QByteArray& buf);
    int writeComxData(const QByteArray& data, bool tryEcho = true);

    QByteArray formatHexText(const QByteArray& buf);
    QString formatUnicodeHexText(const QString& buf);
    QByteArray formatPrintText(const QByteArray& buf);
    int textModelCharCount() const;
    int hexModeCharCount() const;

    bool isHexString(const QByteArray& hex);
    QString formatHexString(const QString& txt);
    void recheckCodePage();

    void handleSplitFilter(QList<LineOutput>& lines);
    void handleOutputFilter(const QByteArray& arrow, const QList<LineOutput>& lines);

    inline QString toHex(QChar c) {
        ushort code = c.unicode();
        uchar *ptr=(uchar*)&code;
        QString tmp = toHex(ptr[0]);
        if(ptr[0] > 0x7F) {
            tmp.append(toHex(ptr[1]));
        }
        return tmp;
    }

    inline QString toHex(uchar c) {
        char tmp[15] = {0};
        sprintf(tmp, "%2.2x ", c);
        return QString(tmp);
    }

    inline QByteArray leftArrow() {
        QByteArray out;
        out.fill('-', 15);
        return "<" + out;
    }

    inline QByteArray rightArrow() {
        QByteArray out;
        out.fill('-', 15);
        return out + ">";
    }
private:
    virtual bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::QWoSerialInput *ui;
    QMap<QString, TimeOutput> m_output;
    QPointer<QWoTermWidget> m_term;
    QPointer<QTcpServer> m_tcpServer;
    QList<QPointer<QTcpSocket>> m_tcpClients;
    QPointer<QTcpSocket> m_tcpClient, m_tcpConnect;
    QPointer<QUdpSocket> m_udpServer, m_udpConnect;
    QPointer<QTimer> m_udpTimer, m_outTimer;
    QMap<QString, qint64> m_udpActived;
    QPointer<QSerialPort> m_serialPort;
    QMap<QByteArray, QByteArray> m_whoBufferLeft;

    QTextCodec *m_codec;
    qint64 m_timeLastSplit;

    bool m_dlgExecInWriteData;
    QByteArray m_fileSendBuffer;
};

#endif // QWOSERIALINPUT_H
