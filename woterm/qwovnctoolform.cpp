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

#include "qwovnctoolform.h"
#include "ui_qwovnctoolform.h"
#include "qwovncwidget.h"
#include "qwofloatwindow.h"
#include "qwomainwindow.h"
#include "qwovncwidgetimpl.h"
#include "qwoshower.h"

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QMenu>
#include <QToolButton>
#include <QStyleOption>

QWoVncToolForm::QWoVncToolForm(QWoVncWidget *vnc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoVncToolForm)
    , m_vnc(vnc)
{
    ui->setupUi(this);
    setObjectName("deskopTool");
    setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(6);
    ui->btnNextScreen->setEnabled(false);
    ui->btnBlack->setEnabled(false);
    ui->btnLock->setEnabled(false);
    ui->btnNextRatio->setEnabled(false);
    ui->btnQuality->setEnabled(false);
    ui->btnFtp->setEnabled(false);
    ui->btnAudioOn->setVisible(false);
    ui->btnAudioOff->setEnabled(false);
    ui->btnAudioOff->setVisible(true);
    ui->btnAudioOn->setVisible(false);

    QObject::connect(m_vnc, SIGNAL(audioStateArrived(int)), this, SLOT(onAudioStateArrived(int)));

    QMenu *menu = new QMenu(this);
    m_qlvMenu = menu;
    QActionGroup *group = new QActionGroup(menu);
    group->setExclusive(true);
    QObject::connect(group, SIGNAL(triggered(QAction*)), this, SLOT(onQualityLevelSelect(QAction*)));
    m_qlv0 = menu->addAction(QIcon(":/resource/skin/signal_1.png"), QObject::tr("Auto mode"));
    m_qlv0->setVisible(false);
    m_qlv1 = menu->addAction(QIcon(":/resource/skin/signal_1.png"), QObject::tr("Lossless mode"));
    m_qlv11 = menu->addAction(QIcon(":/resource/skin/signal_1.png"), QObject::tr("TureColor mode"));
    m_qlv11->setVisible(false);
    m_qlv7 = menu->addAction(QIcon(":/resource/skin/signal_2.png"), QObject::tr("HDTV mode"));
    m_qlv8 = menu->addAction(QIcon(":/resource/skin/signal_3.png"), QObject::tr("General clear mode"));
    m_qlv9 = menu->addAction(QIcon(":/resource/skin/signal_3.png"), QObject::tr("Fast mode"));
    m_qlv10 = menu->addAction(QIcon(":/resource/skin/signal_3.png"), QObject::tr("Super fast mode"));
    m_qlv2 = menu->addAction(QIcon(":/resource/skin/signal_4.png"), QObject::tr("Classics 16bit mode"));
    m_qlv3 = menu->addAction(QIcon(":/resource/skin/signal_5.png"), QObject::tr("Classics 15bit mode"));
    m_qlv4 = menu->addAction(QIcon(":/resource/skin/signal_5.png"), QObject::tr("Classics 8bit mode"));
    m_qlv5 = menu->addAction(QIcon(":/resource/skin/signal_5.png"), QObject::tr("Classics pallete mode"));
    group->addAction(m_qlv0);
    group->addAction(m_qlv1);
    group->addAction(m_qlv2);
    group->addAction(m_qlv3);
    group->addAction(m_qlv4);
    group->addAction(m_qlv5);
    group->addAction(m_qlv7);
    group->addAction(m_qlv8);
    group->addAction(m_qlv9);
    group->addAction(m_qlv10);
    group->addAction(m_qlv11);
    m_qlv0->setCheckable(true);
    m_qlv1->setCheckable(true);
    m_qlv2->setCheckable(true);
    m_qlv3->setCheckable(true);
    m_qlv4->setCheckable(true);
    m_qlv5->setCheckable(true);
    m_qlv7->setCheckable(true);
    m_qlv8->setCheckable(true);
    m_qlv9->setCheckable(true);
    m_qlv10->setCheckable(true);
    m_qlv11->setCheckable(true);
    m_qlv11->setChecked(true);
    ui->btnQuality->setMenu(menu);
    QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(onQualityMenuAboutToShow()));
    QObject::connect(menu, SIGNAL(aboutToHide()), this, SIGNAL(leave()));
    QObject::connect(ui->btnQuality, SIGNAL(clicked(bool)), this, SLOT(onQualityMenuActived()));

    QObject::connect(ui->btnExitFull, SIGNAL(clicked(bool)), this, SLOT(onExitFullScreen()));
    QObject::connect(ui->btnFullScreen, SIGNAL(clicked(bool)), this, SLOT(onFullScreen()));
    QObject::connect(ui->btnMinimize, SIGNAL(clicked(bool)), this, SLOT(onMinimizeWindow()));
    QObject::connect(ui->btnNextRatio, SIGNAL(clicked(bool)), this, SIGNAL(nextScreenRatio()));
    QObject::connect(ui->btnNextScreen, SIGNAL(clicked(bool)), this, SIGNAL(nextScreenMode()));
    QObject::connect(ui->btnBlack, SIGNAL(clicked(bool)), this, SIGNAL(blackScreen()));
    QObject::connect(ui->btnLock, SIGNAL(clicked(bool)), this, SIGNAL(lockScreen()));
    QObject::connect(ui->btnFtp, SIGNAL(clicked(bool)), this, SIGNAL(ftpClicked()));
    QObject::connect(ui->btnReturn, SIGNAL(clicked(bool)), this, SLOT(onReturnTabWindow()));
    QObject::connect(ui->btnClose, SIGNAL(clicked(bool)), this, SLOT(onCloseWindow()));
    QObject::connect(ui->btnAudioOff, SIGNAL(clicked(bool)), this, SLOT(onAudioClicked()));
    QObject::connect(ui->btnAudioOn, SIGNAL(clicked(bool)), this, SLOT(onAudioClicked()));

    parent->installEventFilter(this);

    QObject::connect(QWoMainWindow::shower(), SIGNAL(floatChanged(QWoShowerWidget*,bool)), this, SLOT(onShowerFloatChanged(QWoShowerWidget*,bool)));
}

QWoVncToolForm::~QWoVncToolForm()
{
    delete ui;
}

QSize QWoVncToolForm::sizeHint() const
{
    QSize sz = QWidget::sizeHint();
    return sz;
}

void QWoVncToolForm::onQualityMenuAboutToShow()
{

}

void QWoVncToolForm::onQualityLevelSelect(QAction *pa)
{
    if(pa == m_qlv0) {
        emit qualityLevel(0);
    }else if(pa == m_qlv1) {
        emit qualityLevel(1);
    }else if(pa == m_qlv2) {
        emit qualityLevel(2);
    }else if(pa == m_qlv3) {
        emit qualityLevel(3);
    }else if(pa == m_qlv4) {
        emit qualityLevel(4);
    }else if(pa == m_qlv5) {
        emit qualityLevel(5);
    }else if(pa == m_qlv7) {
        emit qualityLevel(7);
    }else if(pa == m_qlv8) {
        emit qualityLevel(8);
    }else if(pa == m_qlv9) {
        emit qualityLevel(9);
    }else if(pa == m_qlv10) {
        emit qualityLevel(10);
    }else if(pa == m_qlv11) {
        emit qualityLevel(11);
    }
}

/*
 * RGB32_888 = 1, v1
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

   auto, RGB32_888,JPEG_444,H264_High,H264_Normal,
*/
void QWoVncToolForm::onQualityMenuActived()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if(btn != nullptr) {
        QMenu *menu = btn->menu();
        QPoint pt = QCursor::pos();
        QAction* qlvs[] = {m_qlv0,m_qlv1,m_qlv2,m_qlv3,m_qlv4,m_qlv5, nullptr, m_qlv7,m_qlv8,m_qlv9,m_qlv10,m_qlv11};
        QKxVNC::EPixelFormat fmt = m_vnc->getPixelFormat();
        qDebug() << "onQualityMenuActived fmt:" << fmt;
        QAction *lv = qlvs[fmt];
        if(lv) {
            lv->setChecked(true);
        }
        menu->exec(pt);
    }
}

void QWoVncToolForm::onExitFullScreen()
{
    QWidget *w = topLevelWidget();
    w->showNormal();
    QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
}

void QWoVncToolForm::onExitRemoteDesktop()
{
    qint64 pid = QCoreApplication::applicationPid();
    QByteArray _ppid = qgetenv("NETHOST_PROCESS_PID");
    if(pid == _ppid.toLongLong()) {
        QWidget *w = topLevelWidget();
        w->close();
    }else{
        QCoreApplication::quit();
    }
}

void QWoVncToolForm::onReturnTabWindow()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    wfloat->restoreToShower();
    qDebug() << "onReturnTabWindow";
}

void QWoVncToolForm::onFullScreen()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    if(wfloat != nullptr) {
        wfloat->showFullScreen();
        QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
        return;
    }
    QWidget *parent = parentWidget();
    QWoShowerWidget *shower = qobject_cast<QWoShowerWidget*>(parent);
    while(parent != nullptr && shower == nullptr){
        parent = parent->parentWidget();
        shower = qobject_cast<QWoShowerWidget*>(parent);
    }
    if(shower != nullptr) {
        QMetaObject::invokeMethod(QWoMainWindow::shower(), "floatSession", Q_ARG(QWoShowerWidget*, shower), Q_ARG(bool, true));
    }
}

void QWoVncToolForm::onMinimizeWindow()
{
    QWidget *w = topLevelWidget();
    w->showMinimized();
}

void QWoVncToolForm::onScreenCountChanged(int cnt)
{
    bool isWoVNC = m_vnc->isWoVNCServer();
    ui->btnNextScreen->setEnabled(cnt > 1);
    ui->btnBlack->setEnabled(isWoVNC);
    ui->btnLock->setEnabled(isWoVNC);
    ui->btnNextRatio->setEnabled(true);
    ui->btnQuality->setEnabled(isWoVNC);
    QVector<QKxVNC::EEncodingType> encs = m_vnc->encodings();
    bool h264 = encs.contains(QKxVNC::OpenH264);
    m_qlv7->setEnabled(h264);
    m_qlv8->setEnabled(h264);
    m_qlv9->setEnabled(h264);
    m_qlv10->setEnabled(h264);
    resetLayout();
}

void QWoVncToolForm::onMessageSupport(int cnt)
{
    ui->btnFtp->setEnabled(m_vnc->supportMessage(QKxVNC::EMT_Ftp));
    ui->btnAudioOn->setEnabled(m_vnc->supportMessage(QKxVNC::EMT_PlayAudio));
    ui->btnAudioOff->setEnabled(m_vnc->supportMessage(QKxVNC::EMT_PlayAudio));
}

void QWoVncToolForm::onCloseWindow()
{
    QWoShower::forceToCloseTopLevelWidget(this);
}

void QWoVncToolForm::onAudioClicked()
{
    if(ui->btnAudioOff == sender()) {
        m_vnc->setAudioEnabled(true);
    }else{
        m_vnc->setAudioEnabled(false);
    }
}

void QWoVncToolForm::onAudioStateArrived(int state)
{
    ui->btnAudioOff->setVisible(!state);
    ui->btnAudioOn->setVisible(state);
}

void QWoVncToolForm::onShowerFloatChanged(QWoShowerWidget *shower, bool bFloat)
{
    QWoVncWidgetImpl *impl = qobject_cast<QWoVncWidgetImpl*>(shower);
    if(impl == nullptr) {
        return;
    }
    QWidget *topLevel = topLevelWidget();
    topLevel->installEventFilter(this);
    QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
    QWoVncToolForm *form = impl->findChild<QWoVncToolForm*>();
    if(form == this) {
        ui->btnReturn->setVisible(bFloat);
    }
}

void QWoVncToolForm::paintEvent(QPaintEvent *e)
{
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

void QWoVncToolForm::enterEvent(QEvent *e)
{
    qDebug() << "enter";
    emit enter();
}

void QWoVncToolForm::leaveEvent(QEvent *e)
{
    qDebug() << "leave";
    if(m_qlvMenu->isVisible()) {
        return;
    }
    emit leave();
}

bool QWoVncToolForm::eventFilter(QObject *w, QEvent *e)
{
    QEvent::Type t = e->type();
    if(t == QEvent::Resize) {
        QMetaObject::invokeMethod(this, "updateStatus", Qt::QueuedConnection);
    }
    return QWidget::eventFilter(w, e);
}

void QWoVncToolForm::resizeEvent(QResizeEvent *e)
{
    emit sizeChanged(e->size());
    QWidget::resizeEvent(e);
}

void QWoVncToolForm::updateStatus()
{
    QWidget *w = topLevelWidget();
    ui->btnExitFull->setVisible(w->isFullScreen());
    ui->btnFullScreen->setVisible(!w->isFullScreen());
    resetLayout();
}

void QWoVncToolForm::resetLayout()
{
    QSize sz = sizeHint();
    resize(sz);
}
