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

#include "qwoshower.h"
#include "qwosshtermwidgetimpl.h"
#include "qwotelnettermwidgetimpl.h"
#include "qworlogintermwidgetimpl.h"
#include "qwosftpwidgetimpl.h"
#include "qwoserialwidgetimpl.h"
#include "qwovncwidgetimpl.h"
#include "qwordpwidgetimpl.h"
#include "qwoptytermwidgetimpl.h"
#include "qwomainwindow.h"
#include "qwosshconf.h"
#include "qwoevent.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qwofloatwindow.h"
#include "qkxmessagebox.h"
#include "qwobindportpermissiondialog.h"
#include "qkxver.h"
#include "qwosessionproperty.h"

#include <QTabBar>
#include <QResizeEvent>
#include <QtGlobal>
#include <QSplitter>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QProcess>
#include <QVBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QClipboard>

#define FLOAT_WINDOW_TITLE      ("FloatTitle")
#define FLOAT_WINDOW_TOOLBAR    ("FloatToolbar")
#define FLOAT_WINDOW_ICON       ("FloatIcon")

#define TAB_ITEM_DRAG_AWAY_DISTANCE  (100)

QWoShower::QWoShower(QTabBar *tab, QWidget *parent)
    : QStackedWidget (parent)
    , m_tab(tab)
    , m_implCount(0)
{
    m_ptyico = QIcon(":/woterm/resource/skin/console.png");
    m_sshico = QIcon(":/woterm/resource/skin/ssh2.png");
    m_ftpico = QIcon(":/woterm/resource/skin/sftp.png");
    m_telico = QIcon(":/woterm/resource/skin/telnet.png");
    m_rloico = QIcon(":/woterm/resource/skin/rlogin.png");
    m_mtsico = QIcon(":/woterm/resource/skin/mstsc2.png");
    m_serico = QIcon(":/woterm/resource/skin/serialport.png");
    m_vncico = QIcon(":/woterm/resource/skin/vnc2.png");
    QObject::connect(tab, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
    QObject::connect(tab, SIGNAL(currentChanged(int)), this, SLOT(onTabCurrentChanged(int)));
    QObject::connect(tab, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(onTabbarDoubleClicked(int)));
    tab->installEventFilter(this);
}

QWoShower::~QWoShower()
{
    for(auto it = m_floats.begin(); it != m_floats.end(); it++) {
        QWoFloatWindow *wfloat = *it;
        if(wfloat != nullptr) {
            wfloat->close();
        }
    }
}

bool QWoShower::openLocalShell()
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    int gid = QWoUtils::gid();
    QString target = tr("Local shell");
    QWoPtyTermWidgetImpl *impl = new QWoPtyTermWidgetImpl(target, gid, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_LOCALSHELL);
    createTab(impl, m_ptyico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/console.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openScriptRuner(const QString& script)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    return true;
}

bool QWoShower::openSsh(const QString &target, int gid)
{
    if(gid < 0) {
        gid = QWoUtils::gid();
    }
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QWoSshTermWidgetImpl *impl = new QWoSshTermWidgetImpl(target, gid, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_SSH);
    createTab(impl, m_sshico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm-"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/ssh2.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openSftp(const QString &target, int gid)
{
    if(gid < 0) {
        gid = QWoUtils::gid();
    }
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QWoSftpWidgetImpl *impl = new QWoSftpWidgetImpl(target, gid, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_SFTP);
    createTab(impl, m_ftpico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/sftp.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openTelnet(const QString &target)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QWoTelnetTermWidgetImpl *impl = new QWoTelnetTermWidgetImpl(target, QWoUtils::gid(), m_tab, this);
    impl->setProperty("ETabLimitType", ELT_TELNET);
    createTab(impl, m_telico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/telnet.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openRLogin(const QString &target)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    if(QWoUtils::hasUnprivilegedPortPermission()) {
        QWoRLoginTermWidgetImpl *impl = new QWoRLoginTermWidgetImpl(target, QWoUtils::gid(), m_tab, this);
        impl->setProperty("ETabLimitType", ELT_RLOGIN);
        createTab(impl, m_rloico, target);
        impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
        impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
        impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/rlogin.png");
        emit floatChanged(impl, false);
        m_implCount++;
        return true;
    }
    QWoBindPortPermissionDialog dlg(target, this);
    if(dlg.exec() == (QDialog::Accepted+1)) {
        return openRLogin(target);
    }
    return false;
}

bool QWoShower::openMstsc(const QString &target)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QWoRdpWidgetImpl *impl = new QWoRdpWidgetImpl(target, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_RDP);
    createTab(impl, m_mtsico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Mstsc);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/mstsc2.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openVnc(const QString &target)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QWoVncWidgetImpl *impl = new QWoVncWidgetImpl(target, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_VNC);
    createTab(impl, m_vncico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Vnc);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/vnc2.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openSerialPort()
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QString target = tr("SerialPort");
    QWoSerialWidgetImpl *impl = new QWoSerialWidgetImpl(target, QWoUtils::gid(), m_tab, this);
    impl->setProperty("ETabLimitType", ELT_SERIALPORT);
    createTab(impl, m_serico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, ":/woterm/resource/skin/serialport.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::restoreSession(QWoShowerWidget *impl)
{
    impl->setParent(this);
    QString target = impl->targetName();
    QString icon = impl->property(FLOAT_WINDOW_ICON).toString();
    createTab(impl, QIcon(icon), target);
    emit floatChanged(impl, false);

    QTimer::singleShot(1000, this, SLOT(onCleanNilFloatWindow()));
    return true;
}


bool QWoShower::openSsh(const QStringList &targets, int gid)
{
//    if(targets.isEmpty()) {
//        return false;
//    }
//    QStringList mytargets = targets;
//    QString target = mytargets.takeFirst();
//    QWoSshTermWidgetImpl *impl = new QWoSshTermWidgetImpl(target, gid, m_tab, this);
//    impl->joinToVertical(target);
//    createTab(impl, m_sshico, target);
//    int row = targets.length() > 4 ? 3 : 2;
//    for(int r = 1; r < row; r++) {
//        impl->joinToVertical(mytargets.takeFirst());
//    }
//    for(int r = 0; r < row && mytargets.length() > 0; r++) {
//        impl->joinToHorizontal(r, mytargets.takeFirst());
//    }
    return true;
}

void QWoShower::setBackgroundColor(const QColor &clr)
{
    QPalette pal;
    pal.setColor(QPalette::Background, clr);
    pal.setColor(QPalette::Window, clr);
    setPalette(pal);
}

void QWoShower::openFindDialog()
{
    int idx = m_tab->currentIndex();
    if (idx < 0 || idx > m_tab->count()) {
        return;
    }
    QVariant v = m_tab->tabData(idx);
    QWoShowerWidget *target = v.value<QWoShowerWidget*>();
//    QSplitter *take = m_terms.at(idx);
//    Q_ASSERT(target == take);
    //    take->toggleShowSearchBar();
}

int QWoShower::tabCount()
{
    return m_tab->count();
}

void QWoShower::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();
    QRect rt(0, 0, newSize.width(), newSize.height());
}

void QWoShower::syncGeometry(QWidget *widget)
{
    QRect rt = geometry();
    rt.moveTo(0, 0);
    widget->setGeometry(rt);
}

void QWoShower::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect rt(0, 0, width(), height());
    p.fillRect(rt, QColor(Qt::black));
    QFont ft = p.font();
    ft.setPixelSize(100);
    ft.setBold(true);
    p.setFont(ft);
    QPen pen = p.pen();
    pen.setStyle(Qt::DotLine);
    pen.setColor(Qt::lightGray);
    QBrush brush = pen.brush();
    brush.setStyle(Qt::Dense6Pattern);
    pen.setBrush(brush);
    p.setPen(pen);
    p.drawText(rt, Qt::AlignCenter, "WoTerm");
}

bool QWoShower::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type type = ev->type();
    if(type == QEvent::MouseButtonPress) {
        QTabBar* tab = qobject_cast<QTabBar*>(obj);
        if(tab) {
            return tabMouseButtonPress(reinterpret_cast<QMouseEvent*>(ev));
        }
    }else if(type == QEvent::MouseMove) {
        QTabBar* tab = qobject_cast<QTabBar*>(obj);
        if(tab) {
            return tabMouseButtonMove(reinterpret_cast<QMouseEvent*>(ev));
        }
    }
    return false;
}

void QWoShower::closeSession(int idx)
{
    if(idx >= m_tab->count()) {
        return;
    }
    QVariant v = m_tab->tabData(idx);
    QWoShowerWidget *target = v.value<QWoShowerWidget*>();
    QMap<QString, QString> wmsg = target->collectUnsafeCloseMessage();
    QString msg = tr("Close Or Not");
    if(!wmsg.isEmpty()) {
        msg.append(tr("The follow event will be stop before close this session."));
        msg.append("\r\n");
        for(QMap<QString, QString>::iterator iter = wmsg.begin(); iter != wmsg.end(); iter++) {
            msg.append(QString("%1: %2").arg(iter.key()).arg(iter.value()));
        }
        msg.append("\r\n");
        msg.append(tr("Continue To Close It?"));
    }
    QMessageBox::StandardButton btn = QKxMessageBox::warning(this, tr("CloseSession"), msg, QMessageBox::Ok|QMessageBox::No);
    if(btn == QMessageBox::No) {
        return ;
    }
    target->deleteLater();
}

void QWoShower::createTab(QWoShowerWidget *impl, const QIcon&icon, const QString& tabName)
{
    addWidget(impl);
    int idx = m_tab->addTab(icon, tabName);
    m_tab->setCurrentIndex(idx);
    m_tab->setTabData(idx, QVariant::fromValue(impl));
    QObject::connect(impl, SIGNAL(destroyed(QObject*)), this, SLOT(onTabWidgetDestroy(QObject*)), Qt::UniqueConnection);
    setCurrentWidget(impl);
    qDebug() << "tabCount" << m_tab->count() << ",implCount" << count();
}

bool QWoShower::tabMouseButtonPress(QMouseEvent *ev)
{
    m_tabDragAway = false;
    m_tabDragWidget = nullptr;
    QPoint pt = ev->pos();
    int idx = m_tab->tabAt(pt);
    if(idx < 0) {
        onTabContextMenu(ev);
        return false;
    }
    qDebug() << "tab hit" << idx;
    QVariant v = m_tab->tabData(idx);
    QWoShowerWidget *impl = v.value<QWoShowerWidget*>();
    if(ev->buttons().testFlag(Qt::RightButton)) {
        QMenu menu(impl);
        m_tabMenu = &menu;
        m_tabMenu->setProperty(TAB_TARGET_IMPL, QVariant::fromValue(impl));
        menu.addAction(tr("Close this tab"), this, SLOT(onCloseThisTabSession()));
        menu.addAction(tr("Close other tab"), this, SLOT(onCloseOtherTabSession()));
        menu.addAction(tr("New same session"), this, SLOT(onNewTheSameSession()));
        menu.addAction(tr("Copy host address"), this, SLOT(onCopyTabSessionAddress()));
        menu.addAction(tr("Edit this session"), this, SLOT(onEditThisTabSession()));
        menu.addAction(tr("Float this tab"), this, SLOT(onFloatThisTabSession()));
        impl->handleTabContextMenu(&menu);
        menu.exec(QCursor::pos());
    }else if(ev->buttons().testFlag(Qt::LeftButton)){
        m_tabDragWidget = impl;
    }
    return false;
}

bool QWoShower::tabMouseButtonMove(QMouseEvent *ev)
{
    QPoint pt = ev->pos();
    if(ev->buttons().testFlag(Qt::LeftButton)) {
        if(pt.y() > TAB_ITEM_DRAG_AWAY_DISTANCE) {
            //qDebug() << "tabMouseButtonMove" << pt;
            if(m_tabDragWidget) {
                floatSession(m_tabDragWidget, false);
                m_tabDragWidget = nullptr;
            }
        }
    }
    return false;
}

void QWoShower::onTabCloseRequested(int idx)
{
    closeSession(idx);
}

void QWoShower::onTabCurrentChanged(int idx)
{
    if(idx < 0) {
        return;
    }
    QVariant v = m_tab->tabData(idx);
    if(v.isValid()) {
        QWoShowerWidget *impl = v.value<QWoShowerWidget *>();
        setCurrentWidget(impl);
    }
}

void QWoShower::onTabWidgetDestroy(QObject *it)
{
    QWidget *target = qobject_cast<QWidget*>(it);
    m_implCount--;
    for(int i = 0; i < m_tab->count(); i++) {
        QVariant v = m_tab->tabData(i);
        QWidget *impl = v.value<QWidget *>();
        if(target == impl) {            
            removeWidget(target);
            m_tab->removeTab(i);
            break;
        }
    }
    onCleanNilFloatWindow();
    qDebug() << "onTabWidgetDestroy: tabCount" << m_tab->count() << ",implCount" << count();
}

void QWoShower::onTabbarDoubleClicked(int index)
{
    if(index < 0 && QKxVer::instance()->isFullFeather()) {
        openLocalShell();
    }
}

void QWoShower::onCloseThisTabSession()
{
    if(m_tabMenu == nullptr) {
        return;
    }
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWidget *impl = vimpl.value<QWidget*>();
    if(impl == nullptr) {
        QKxMessageBox::warning(this, tr("alert"), tr("failed to find impl infomation"));
        return;
    }
    impl->deleteLater();
}

void QWoShower::onCloseOtherTabSession()
{
    if(m_tabMenu == nullptr) {
        return;
    }
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWidget *impl = vimpl.value<QWidget*>();
    if(impl == nullptr) {
        QKxMessageBox::warning(this, tr("alert"), tr("failed to find impl infomation"));
        return;
    }
    for(int i = 0; i < m_tab->count(); i++) {
        QVariant v = m_tab->tabData(i);
        QWidget *target = v.value<QWidget *>();
        if(target != impl) {
            target->deleteLater();
        }
    }
}

void QWoShower::onNewTheSameSession()
{
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWoShowerWidget *impl = vimpl.value<QWoShowerWidget*>();
    QString target = impl->targetName();
    ELimitType t = static_cast<ELimitType>(impl->property("ETabLimitType").toInt());
    if(t == ELT_LOCALSHELL) {
        openLocalShell();
    }else if(t == ELT_SSH) {
        openSsh(target);
    }else if(t == ELT_SFTP) {
        openSftp(target);
    }else if(t == ELT_TELNET) {
        openTelnet(target);
    }else if(t == ELT_RLOGIN) {
        openRLogin(target);
    }else if(t == ELT_RDP) {
        openMstsc(target);
    }else if(t == ELT_VNC) {
        openVnc(target);
    }else if(t == ELT_SERIALPORT) {
        openSerialPort();
    }
}

void QWoShower::onCopyTabSessionAddress()
{
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWoShowerWidget *impl = vimpl.value<QWoShowerWidget*>();
    QString target = impl->targetName();
    const HostInfo& hi = QWoSshConf::instance()->find(target);
    if(hi.isValid()) {
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(hi.host);
    }
}

void QWoShower::onEditThisTabSession()
{
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWoShowerWidget *impl = vimpl.value<QWoShowerWidget*>();
    impl->handleCustomProperties();
}

void QWoShower::onFloatThisTabSession()
{
    if(m_tabMenu == nullptr) {
        return;
    }
    QVariant vimpl = m_tabMenu->property(TAB_TARGET_IMPL);
    QWoShowerWidget *impl = vimpl.value<QWoShowerWidget*>();

    floatSession(impl, false);
}

void QWoShower::floatSession(QWoShowerWidget *impl, bool full)
{
    for(int i = 0; i < m_tab->count(); i++) {
        QVariant v = m_tab->tabData(i);
        QWoShowerWidget *hit = v.value<QWoShowerWidget*>();
        if(hit == impl) {
            removeWidget(hit);
            m_tab->removeTab(i);
            break;
        }
    }
    QString icon = impl->property(FLOAT_WINDOW_ICON).toString();
    QString title = impl->property(FLOAT_WINDOW_TITLE).toString();
    QWoFloatWindow::EToolType typ = impl->property(FLOAT_WINDOW_TOOLBAR).value<QWoFloatWindow::EToolType>();
    QWoFloatWindow *wfloat = new QWoFloatWindow(impl, this, typ);
    wfloat->setWindowTitle(title);
    wfloat->setWindowIcon(QIcon(icon));
    QRect rtImpl = rect();
    QPoint pt = mapToGlobal(QPoint(0, 0));
    rtImpl.moveTo(pt);
    rtImpl.translate(50, 50);
    wfloat->setGeometry(rtImpl);
    if(full) {
        wfloat->showFullScreen();
    }else{
        wfloat->show();
        wfloat->raise();
    }
    m_floats.append(wfloat);
    emit floatChanged(impl, true);
}

void QWoShower::forceToCloseTopLevelWidget(QWidget *child)
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(child->topLevelWidget());
    if(wfloat != nullptr) {
        wfloat->close();
        return;
    }
    QWoShowerWidget *shower = qobject_cast<QWoShowerWidget*>(child);
    if(shower != nullptr) {
        shower->close();
        return;
    }
    QWidget *parent = child->parentWidget();
    shower = qobject_cast<QWoShowerWidget*>(parent);
    while(parent != nullptr && shower == nullptr){
        parent = parent->parentWidget();
        shower = qobject_cast<QWoShowerWidget*>(parent);
    }
    if(shower != nullptr) {
        shower->close();
    }
}

void QWoShower::onCleanNilFloatWindow()
{
    for(auto it = m_floats.begin(); it != m_floats.end();) {
        QWoFloatWindow* wfloat = *it;
        if(wfloat == nullptr) {
            it = m_floats.erase(it);
        }else{
            it++;
        }
    }
}

void QWoShower::onTabContextMenu(QMouseEvent *ev)
{
    if(ev->buttons() & Qt::RightButton) {
        if(QKxVer::instance()->isFullFeather()) {
            QMenu menu;
            menu.addAction(QIcon(":/woterm/resource/skin/nodes.png"), tr("Open remote session"), this, SLOT(onOpenRemoteSession()));
            menu.addAction(QIcon(":/woterm/resource/skin/console.png"), tr("Open local session"), this, SLOT(onOpenLocalSession()));
            menu.exec(QCursor::pos());
        }
    }
}

void QWoShower::onOpenRemoteSession()
{
    QMetaObject::invokeMethod(QWoMainWindow::instance(), "onOpenRemoteSession", Qt::QueuedConnection);
}

void QWoShower::onOpenLocalSession()
{
    openLocalShell();
}
