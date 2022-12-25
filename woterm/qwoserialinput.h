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
class QWoSerialInput : public QWidget
{
    Q_OBJECT
public:
    explicit QWoSerialInput(QWoTermWidget *term, QWidget *parent = 0);
    ~QWoSerialInput();
    void reset();
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
    void onComxError();

    void onTcpListenButtonClicked();
    void onTcpConnectButtonClicked();
    void onUdpListenButtonClicked();
    void onUdpStartButtonClicked();
    void onComxConnectButtonClicked();
    void onTextSendButtonClicked();
    void onSerialPortCurrentTextChanged(const QString& portName);
    void onInputAsHexClicked();
    void onOutputAsHexClicked();
    void onEditTextChanged();
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
    void parse(const QByteArray& arrow, const QByteArray& who, const QList<QByteArray>& lines);
    QList<QByteArray> formatText(const QByteArray& buf);
    QList<QByteArray> formatHexText(const QByteArray& buf);
    QList<QByteArray> formatTextText(const QByteArray& buf);
    int hexModeCharCount() const;
    int textModelCharCount() const;

    bool isHexString(const QByteArray& hex);
    QString formatHexString(const QString& txt);
private:
    virtual bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::QWoSerialInput *ui;
    QPointer<QWoTermWidget> m_term;
    QPointer<QTcpServer> m_tcpServer;
    QList<QPointer<QTcpSocket>> m_tcpClients;
    QPointer<QTcpSocket> m_tcpClient, m_tcpConnect;
    QPointer<QUdpSocket> m_udpServer, m_udpConnect;
    QPointer<QTimer> m_udpTimer;
    QMap<QString, qint64> m_udpActived;
    QPointer<QSerialPort> m_serialPort;
};

#endif // QWOSERIALINPUT_H
