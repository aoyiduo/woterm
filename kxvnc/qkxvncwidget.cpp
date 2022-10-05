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

#include "qkxvncwidget.h"
#include "qkxvnc.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMetaObject>
#include <QTimer>
#include <QDebug>
#include <QClipboard>
#include <QGuiApplication>
#include <QScrollArea>

QKxVncWidget::QKxVncWidget(QWidget *parent)
    : QWidget(parent)
{
    m_screenIndex = 0;
    m_screenCount = 0;
    m_qualityLevel = 2;
    m_viewOnly = false;
    m_scrollEnabled = true;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setInputMethodHints((Qt::InputMethodHints)(Qt::ImhNoAutoUppercase|Qt::ImhNoPredictiveText));

    //QTimer *timer = new QTimer(this);
    //QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    //timer->start(30);

    QImage tmp = QImage(5, 5, QImage::Format_ARGB32_Premultiplied);
    tmp.fill(qRgba(0, 0, 0, 0));
    m_hideCursor = QCursor(QPixmap::fromImage(tmp), 2, 2);
    tmp.setPixel(0, 2, qRgba(0, 0, 0, 255));
    tmp.setPixel(1, 1, qRgba(0, 0, 0, 255));
    tmp.setPixel(1, 3, qRgba(0, 0, 0, 255));
    tmp.setPixel(2, 0, qRgba(0, 0, 0, 255));
    tmp.setPixel(2, 4, qRgba(0, 0, 0, 255));
    tmp.setPixel(3, 1, qRgba(0, 0, 0, 255));
    tmp.setPixel(3, 3, qRgba(0, 0, 0, 255));
    tmp.setPixel(4, 2, qRgba(0, 0, 0, 255));
    tmp.setPixel(1, 2, qRgba(255, 255, 255, 255));
    tmp.setPixel(2, 1, qRgba(255, 255, 255, 255));
    tmp.setPixel(2, 2, qRgba(255, 255, 255, 255));
    tmp.setPixel(2, 3, qRgba(255, 255, 255, 255));
    tmp.setPixel(3, 2, qRgba(255, 255, 255, 255));
    m_showCursor = QCursor(QPixmap::fromImage(tmp), 2, 2);
    setLocalCursorVisible(true);

    QClipboard *clipboard = QGuiApplication::clipboard();
    QObject::connect(clipboard, SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));

    m_area = qobject_cast<QScrollArea*>(parent);
    if(m_area){
        m_area->installEventFilter(this);
    }
}

QKxVncWidget::~QKxVncWidget()
{
    if(m_vnc) {
        QKxVNCFactory::instance()->release(m_vnc);
    }
}

void QKxVncWidget::start(const QString& host, int port, const QString& passwd, QKxVNC::EPixelFormat fmt, QKxVNC::EProtoVersion proto, QVector<QKxVNC::EEncodingType> enc)
{
    if(m_vnc) {
        QKxVNCFactory::instance()->release(m_vnc);
    }
    m_encs = enc;
    m_screenCount = 0;
    m_vnc = QKxVNCFactory::instance()->create();
    QObject::connect(m_vnc, SIGNAL(updateArrived()), this, SLOT(onUpdateArrived()));
    QObject::connect(m_vnc, SIGNAL(cutTextArrived(QString)), this, SLOT(onServerCutTextArrived(QString)));
    QObject::connect(m_vnc, SIGNAL(finished()), this, SLOT(onVncFinished()));
    QObject::connect(m_vnc, SIGNAL(screenCountChanged(int)), this, SLOT(onScreenCountChanged(int)));
    QObject::connect(m_vnc, SIGNAL(messageSupport(int)), this, SLOT(onMessageSupport(int)));
    QObject::connect(m_vnc, SIGNAL(ftpArrived(QByteArray)), this, SLOT(onFtpResult(QByteArray)));
    // signals -> signals;
    QObject::connect(m_vnc, SIGNAL(qpsArrived(int)), this, SIGNAL(qpsArrived(int)));
    QObject::connect(m_vnc, SIGNAL(audioStateArrived(int)), this, SIGNAL(audioStateArrived(int)));
    QObject::connect(m_vnc, SIGNAL(privacyStateArrived(int)), this, SIGNAL(privacyStateArrived(int)));

    QObject::connect(m_vnc, SIGNAL(connectionStart()), this, SIGNAL(connectionStart()));
    QObject::connect(m_vnc, SIGNAL(connectionFinished(bool)), this, SIGNAL(connectionFinished(bool)));
    QObject::connect(m_vnc, SIGNAL(errorArrived(QByteArray)), this, SIGNAL(errorArrived(QByteArray)));
    QObject::connect(m_vnc, SIGNAL(passwordResult(QByteArray,bool)), this, SIGNAL(passwordResult(QByteArray,bool)));
    QObject::connect(m_vnc, SIGNAL(inputArrived(QString,bool)), this, SIGNAL(inputArrived(QString,bool)));
    m_vnc->start(host.toLatin1(), port, passwd.toLatin1(), fmt, proto, enc);
}

void QKxVncWidget::setViewOnly(bool on)
{
    m_viewOnly = on;
}

void QKxVncWidget::setLocalCursorVisible(bool on)
{
    setCursor(on ? m_showCursor : m_hideCursor);
}

void QKxVncWidget::setInputResult(const QString &passwd)
{
    if(m_vnc) {
        m_vnc->setInputResult(passwd);
    }
}

bool QKxVncWidget::scrollEnabled()
{
    if(m_area == nullptr) {
        return false;
    }
    return m_scrollEnabled;
}

void QKxVncWidget::setScrollEnabled(bool on)
{
    if(m_area == nullptr) {
        return;
    }
    m_scrollEnabled = on;
    QMetaObject::invokeMethod(this, "updateRatio", Qt::QueuedConnection);
}

void QKxVncWidget::setNextRatio()
{
    m_scrollEnabled = !m_scrollEnabled;
    QMetaObject::invokeMethod(this, "updateRatio", Qt::QueuedConnection);
}

void QKxVncWidget::setNextScreen()
{
    if(m_screenCount > 0) {
        m_screenIndex++;
        m_screenIndex %= (m_screenCount+1);
        QMetaObject::invokeMethod(this, "updateRatio", Qt::QueuedConnection);
    }
}

void QKxVncWidget::setAudioEnabled(bool on)
{
    if(m_vnc != nullptr) {
        m_vnc->setAudioEnabled(on);
    }
}

void QKxVncWidget::setPrivacyScreenEnabled(bool on)
{
    if(m_vnc != nullptr) {
        m_vnc->setPrivacyScreenEnabled(on);
    }
}

int QKxVncWidget::remoteScreenCount()
{
    if(m_vnc != nullptr) {
        return 1;
    }
    return m_vnc->screenCount();
}

QSize QKxVncWidget::remoteDesktopSize()
{
    QImage img = capture();
    return img.size();
}

QString QKxVncWidget::hostPort() const
{
    if(m_vnc!=nullptr){
        return m_vnc->hostPort();
    }
    return QString();
}

bool QKxVncWidget::isConnected()
{
    return m_screenCount > 0;
}

void QKxVncWidget::setBlackScreen()
{
    if(m_vnc && m_vnc->supportBlackScreen()) {
        m_vnc->setBlackScreen();
    }
}

void QKxVncWidget::setLockScreen()
{
    if(m_vnc && m_vnc->supportLockScreen()) {
        m_vnc->setLockScreen();
    }
}

void QKxVncWidget::setQualityLevel(int lv)
{
    m_qualityLevel = lv;
    if(m_vnc == nullptr) {
        return;
    }
    switch (lv) {
    case 1:
        m_vnc->setPixelFormat(QKxVNC::RGB32_888);
        break;
    case 2:
        m_vnc->setPixelFormat(QKxVNC::RGB16_565);
        break;
    case 3:
        m_vnc->setPixelFormat(QKxVNC::RGB15_555);
        break;
    case 4:
        m_vnc->setPixelFormat(QKxVNC::RGB8_332);
        break;
    case 5:
        m_vnc->setPixelFormat(QKxVNC::RGB8_Map);
        break;
    case 7:
        m_vnc->setPixelFormat(QKxVNC::H264_High);
        break;
    case 8:
        m_vnc->setPixelFormat(QKxVNC::H264_Normal);
        break;
    case 9:
        m_vnc->setPixelFormat(QKxVNC::H264_Low);
        break;
    case 10:
        m_vnc->setPixelFormat(QKxVNC::H264_Lowest);
        break;
    case 11:
        m_vnc->setPixelFormat(QKxVNC::JPEG_444);
        break;
    default:
        break;
    }
    qDebug() << "setQualityLevel" << lv;
}

bool QKxVncWidget::isWoVNCServer() const
{
    return m_vnc->isWoVNCServer();
}

bool QKxVncWidget::isAudioPlay() const
{
    return m_vnc->isAudioPlay();
}

QKxVNC::EPixelFormat QKxVncWidget::getPixelFormat() const
{
    if(m_vnc) {
        return m_vnc->getPixelFormat();
    }
    return QKxVNC::RGB16_565;
}

QVector<QKxVNC::EEncodingType> QKxVncWidget::encodings()
{
    return m_encs;
}

bool QKxVncWidget::supportMessage(QKxVNC::EMessageType emt) const
{
    if(m_vnc) {
        return m_vnc->supportMessage(emt);
    }
    return false;
}

bool QKxVncWidget::sendFtpPacket(const QByteArray &data)
{
    if(m_vnc) {
        return m_vnc->sendFtpPacket(data);
    }
    return false;
}

void QKxVncWidget::onFtpResult(const QByteArray &buf)
{
    emit ftpArrived(buf);
}

void QKxVncWidget::onMessageSupport(int cnt)
{
    qDebug() << "onMessageSupport" << cnt;
    emit messageSupport(cnt);
}

void QKxVncWidget::onScreenCountChanged(int cnt)
{
    m_screenCount = cnt;
    updateRatio();
    emit screenCountChanged(cnt);
}

void QKxVncWidget::onVncFinished()
{
    emit finished();
    QKxVNCFactory::instance()->release(m_vnc);
}

void QKxVncWidget::onTimeout()
{
    if(m_vnc) {
        update(rect());
    }
}

void QKxVncWidget::onUpdateArrived()
{
    updateNow();
}

void QKxVncWidget::onClipboardDataChanged()
{
    if(m_vnc) {
        QClipboard *clipboard = qobject_cast<QClipboard*>(sender());
        QString txt = clipboard->text();
        if(txt != m_cutTextLast){
            m_vnc->sendClientCutText(txt);
        }
    }
}

void QKxVncWidget::onServerCutTextArrived(const QString &txt)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    m_cutTextLast = txt;
    if(txt != clipboard->text()) {
        clipboard->setText(txt);
    }
}

void QKxVncWidget::onUpdateRatio()
{
    updateRatio();
}

void QKxVncWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setClipRegion(ev->region());
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QRect drawRt = rect();
    if(m_vnc) {
        QImage img = m_vnc->capture();
        if(img.isNull()) {
            p.fillRect(drawRt, Qt::black);
        }else{
            QRect rt = m_vnc->screenRect(m_screenIndex);
            p.drawImage(drawRt, img, rt);
        }
    }else{
        p.fillRect(drawRt, Qt::black);
    }
}

void QKxVncWidget::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "mousePressEvent" << event->buttons();
    if(m_vnc && !m_viewOnly) {
        QRect rt =  m_vnc->screenRect(m_screenIndex);
        m_vnc->handleMouseEvent(event, size(), rt);
        updateNow();
    }
}

void QKxVncWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouseMoveEvent" << event->buttons();
    if(m_vnc && !m_viewOnly) {
        QRect rt =  m_vnc->screenRect(m_screenIndex);
        m_vnc->handleMouseEvent(event, size(), rt);
        updateNow();
    }
}

void QKxVncWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "mouseReleaseEvent" << event->buttons();
    if(m_vnc && !m_viewOnly) {
        QRect rt =  m_vnc->screenRect(m_screenIndex);
        m_vnc->handleMouseEvent(event, size(), rt);
        updateNow();
    }
}

void QKxVncWidget::wheelEvent(QWheelEvent *event)
{
    //qDebug() << "wheelEvent" << event->buttons() << event->delta();
    if(m_vnc && !m_viewOnly) {
        m_vnc->wheelEvent(event, size());
        updateNow();
    }
}

void QKxVncWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keyPressEvent" << event->key();
    if(m_vnc && !m_viewOnly) {
        m_vnc->keyPressEvent(event);
        updateNow();
    }
}

void QKxVncWidget::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "keyReleaseEvent" << event->key();
    if(m_vnc && !m_viewOnly) {
        m_vnc->keyReleaseEvent(event);
        updateNow();
    }
}

bool QKxVncWidget::focusNextPrevChild(bool next)
{
    return false;
}

void QKxVncWidget::inputMethodEvent(QInputMethodEvent *event)
{
     const QString& commit = event->commitString();
    if (!commit.isEmpty()) {
        int key = 0;
        if (commit.size() > 1)
        {
            for (int i = 0; i < commit.size(); ++i) {
                QKeyEvent ev(QEvent::KeyPress, key, Qt::NoModifier, commit[i]);
                keyPressEvent(&ev);
                keyReleaseEvent(&ev);
            }
        }
        else
        {
            if (commit[0].isLetter()){
                key = commit[0].toUpper().toLatin1();
            }
            QKeyEvent ev(QEvent::KeyPress, key, Qt::NoModifier, commit[0]);
            keyPressEvent(&ev);
            keyReleaseEvent(&ev);
        }
    }
    event->accept();
}

QVariant QKxVncWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return 0;
}

void QKxVncWidget::focusInEvent(QFocusEvent *ev)
{
    QWidget::focusInEvent(ev);
    if(m_vnc) {
        m_vnc->restoreKeyboardStatus();
    }
    updateNow();
}

void QKxVncWidget::focusOutEvent(QFocusEvent *ev)
{
    QWidget::focusOutEvent(ev);
    if(m_vnc) {
        m_vnc->restoreKeyboardStatus();
    }
    updateNow();
}

bool QKxVncWidget::eventFilter(QObject *w, QEvent *e)
{
    QEvent::Type t = e->type();
    if(t == QEvent::Resize) {
        QScrollArea *area = qobject_cast<QScrollArea*>(w);
        if(area != nullptr) {
            QMetaObject::invokeMethod(this, "updateRatio", Qt::QueuedConnection);
        }
    }
    return QWidget::eventFilter(w, e);
}

void QKxVncWidget::updateNow()
{
    if(m_vnc) {
        QRect imgRt = m_vnc->screenRect(m_screenIndex);
        QRect rt = m_vnc->clip(size(), imgRt);
        if(!rt.isEmpty()){
            update(rt);
        }
    }
}

QImage QKxVncWidget::capture() const
{
    if(m_vnc) {
        return m_vnc->capture();
    }
    return QImage();
}

void QKxVncWidget::updateRatio()
{
    if(m_area == nullptr || m_vnc == nullptr ) {
        return;
    }
    if(m_scrollEnabled) {
        QRect rt = m_vnc->screenRect(m_screenIndex);
        resize(rt.size());
    }else{
        QRect rt = m_area->rect();
        resize(rt.size());
    }
    update();
}
