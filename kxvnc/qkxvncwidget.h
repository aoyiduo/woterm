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

#ifndef QKXVNCWIDGET_H
#define QKXVNCWIDGET_H

#include "qkxvnc_share.h"
#include "qkxvnc.h"

#include <QWidget>
#include <QPointer>

class QScrollArea;
class QKXVNC_EXPORT QKxVncWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QKxVncWidget(QWidget*parent=nullptr);
    ~QKxVncWidget();
    Q_INVOKABLE void start(const QString& host, int port, const QString& passwd, QKxVNC::EPixelFormat fmt = QKxVNC::RGB16_565, QKxVNC::EProtoVersion proto = QKxVNC::RFB_38, QVector<QKxVNC::EEncodingType> enc = QVector<QKxVNC::EEncodingType>());
    void setViewOnly(bool on);
    void setLocalCursorVisible(bool on);
    void setInputResult(const QString& passwd);
    bool scrollEnabled();
    void setScrollEnabled(bool on);
    void setNextRatio();
    void setNextScreen();
    void setAudioEnabled(bool on);
    void setPrivacyScreenEnabled(bool on);
    int remoteScreenCount();
    QSize remoteDesktopSize();
    QString hostPort() const;
    bool isConnected();
    void setBlackScreen();
    void setLockScreen();
    void setQualityLevel(int lv);
    bool isWoVNCServer() const;
    bool isAudioPlay() const;
    QKxVNC::EPixelFormat getPixelFormat() const;
    QVector<QKxVNC::EEncodingType> encodings();
    bool supportMessage(QKxVNC::EMessageType emt) const;
    bool sendFtpPacket(const QByteArray& data);
signals:
    void qpsArrived(int qps);
    void ftpArrived(const QByteArray& buf);
    void audioStateArrived(int state);
    void privacyStateArrived(int state);
    void messageSupport(int cnt);
    void screenCountChanged(int cnt);
    void connectionStart();
    void connectionFinished(bool ok);
    void finished();
    void errorArrived(const QByteArray& buf);
    void passwordResult(const QByteArray& passwd, bool ok);
    void inputArrived(const QString& prompt, bool visible);
private slots:
    void onFtpResult(const QByteArray& buf);
    void onMessageSupport(int cnt);
    void onScreenCountChanged(int cnt);
    void onVncFinished();
    void onTimeout();
    void onUpdateArrived();
    void onClipboardDataChanged();
    void onServerCutTextArrived(const QString& txt);
    void onUpdateRatio();
protected:
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual bool focusNextPrevChild(bool next);
    virtual void inputMethodEvent(QInputMethodEvent *ev);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    virtual void focusInEvent(QFocusEvent *ev);
    virtual void focusOutEvent(QFocusEvent *ev);
    virtual bool eventFilter(QObject *w, QEvent *e);
protected:
    void updateNow();
    QImage capture() const;
    Q_INVOKABLE void updateRatio();
protected:
    QPointer<QKxVNC> m_vnc;
private:
    QCursor m_showCursor, m_hideCursor;
    bool m_capson;
    bool m_viewOnly;
    QString m_cutTextLast;
    bool m_scrollEnabled;
    int m_screenIndex;
    int m_screenCount;
    int m_qualityLevel;
    QVector<QKxVNC::EEncodingType> m_encs;
    QPointer<QScrollArea> m_area;
};

#endif // QKXVNCWIDGET_H
