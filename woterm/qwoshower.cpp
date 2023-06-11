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
#include "qwoplaybookwidget.h"
#include "qwoplaybookwidgetimpl.h"

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
    m_jsico = QIcon("../private/skins/black/javascript.png");
    m_bookico = QIcon("../private/skins/black/js.png");
    m_ptyico = QIcon("../private/skins/black/console.png");
    m_sshico = QIcon("../private/skins/black/ssh2.png");
    m_ftpico = QIcon("../private/skins/black/sftp.png");
    m_telico = QIcon("../private/skins/black/telnet.png");
    m_rloico = QIcon("../private/skins/black/rlogin.png");
    m_mtsico = QIcon("../private/skins/black/mstsc2.png");
    m_serico = QIcon("../private/skins/black/serialport.png");
    m_vncico = QIcon("../private/skins/black/vnc2.png");
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

bool QWoShower::openPlayBook(const QString& name, const QString &path)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QString target = tr("PB")+":"+name;
    QWoPlaybookWidgetImpl *impl = new QWoPlaybookWidgetImpl(path, target, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_PLAYBOOK);
    createTab(impl, m_bookico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Common);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/js.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

bool QWoShower::openLocalShell()
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    QString target = tr("Local shell");
    QWoPtyTermWidgetImpl *impl = new QWoPtyTermWidgetImpl(target, QWoUtils::gid(), m_tab, this);
    impl->setProperty("ETabLimitType", ELT_LOCALSHELL);
    createTab(impl, m_ptyico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/console.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

void QWoShower::attachLocalShell(QWoTermWidget *term, int idx)
{
    QString target = term->target();
    int gid = term->gid();
    QWoPtyTermWidgetImpl *impl = new QWoPtyTermWidgetImpl(target, gid, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_LOCALSHELL);
    impl->setForCreated(term);
    createTab(impl, m_ptyico, target, idx);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/console.png");
    emit floatChanged(impl, false);
    m_implCount++;
}

bool QWoShower::openSsh(const QString &target, int gid)
{
    if(m_implCount >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The tab's count has reached the limit."));
        return false;
    }
    if(gid < 0) {
        gid = QWoUtils::gid();
    }
    QWoSshTermWidgetImpl *impl = new QWoSshTermWidgetImpl(target, gid, m_tab, this);
    impl->setProperty("ETabLimitType", ELT_SSH);
    createTab(impl, m_sshico, target);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/ssh2.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

void QWoShower::attachSsh(QWoTermWidget *term, int idx)
{
    QString target = term->target();
    int gid = term->gid();
    QWoSshTermWidgetImpl *impl = new QWoSshTermWidgetImpl(target, gid, m_tab, this);
    impl->setForCreated(term);
    impl->setProperty("ETabLimitType", ELT_SSH);
    createTab(impl, m_sshico, target, idx);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/ssh2.png");
    emit floatChanged(impl, false);
    m_implCount++;
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
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/sftp.png");
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
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/telnet.png");
    emit floatChanged(impl, false);
    m_implCount++;
    return true;
}

void QWoShower::attachTelnet(QWoTermWidget *term, int idx)
{
    QString target = term->target();
    int gid = term->gid();
    QWoTelnetTermWidgetImpl *impl = new QWoTelnetTermWidgetImpl(target, gid, m_tab, this);
    impl->setForCreated(term);
    impl->setProperty("ETabLimitType", ELT_TELNET);
    createTab(impl, m_telico, target, idx);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/telnet.png");
    emit floatChanged(impl, false);
    m_implCount++;
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
        impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/rlogin.png");
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

void QWoShower::attachRLogin(QWoTermWidget *term, int idx)
{
    QString target = term->target();
    int gid = term->gid();
    QWoRLoginTermWidgetImpl *impl = new QWoRLoginTermWidgetImpl(target, gid, m_tab, this);
    impl->setForCreated(term);
    impl->setProperty("ETabLimitType", ELT_RLOGIN);
    createTab(impl, m_rloico, target, idx);
    impl->setProperty(FLOAT_WINDOW_TOOLBAR, QWoFloatWindow::ETT_Term);
    impl->setProperty(FLOAT_WINDOW_TITLE, "WoTerm:"+target);
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/rlogin.png");
    emit floatChanged(impl, false);
    m_implCount++;
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
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/mstsc2.png");
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
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/vnc2.png");
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
    impl->setProperty(FLOAT_WINDOW_ICON, "../private/skins/black/serialport.png");
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
    if (idx < 0 || idx >= m_tab->count()) {
        return;
    }
    QVariant v = m_tab->tabData(idx);
    QWoShowerWidget *target = v.value<QWoShowerWidget*>();
}

void QWoShower::mergeFromRightTab()
{
    int idx = m_tab->currentIndex();
    if (idx < 0 || idx >= m_tab->count()) {
        return;
    }
    if(idx == m_tab->count() - 1) {
        QKxMessageBox::information(this, tr("Merge information"), tr("There are no right tab to merge."));
        return;
    }

    QVariant vImpl = m_tab->tabData(idx);
    QWoTermWidgetImpl *impl = vImpl.value<QWoTermWidgetImpl*>();
    if(impl == nullptr) {
        QKxMessageBox::information(this, tr("Merge information"), tr("Current tab is not a text terminal tab."));
        return;
    }

    QVariant vImplRight = m_tab->tabData(idx+1);
    QWoTermWidgetImpl *implRight = vImplRight.value<QWoTermWidgetImpl*>();
    if(implRight == nullptr) {
        QKxMessageBox::information(this, tr("Merge information"), tr("Cannot merge because the right tab is not a text terminal."));
        return;
    }
    QList<QPointer<QWoTermWidget>> terms = implRight->termAll();
    if(impl->termCount() + terms.length() > 8) {
        QKxMessageBox::information(this, tr("Merge information"), tr("The number of merging has exceeded 8 and cannot be continued."));
        return;
    }
    for(auto it = terms.begin(); it != terms.end(); it++) {
        QWoTermWidget *term = *it;
        if(term == nullptr) {
            continue;
        }
        term->detachWidget();
        impl->attachBy(term);
    }
    if(implRight->termCount() == 0) {
        implRight->deleteLater();
    }
    m_tab->setCurrentIndex(idx);
}

void QWoShower::seperateToRightTab()
{
    int idx = m_tab->currentIndex();
    if (idx < 0 || idx >= m_tab->count()) {
        return;
    }

    QVariant vImpl = m_tab->tabData(idx);
    QWoTermWidgetImpl *impl = vImpl.value<QWoTermWidgetImpl*>();
    if(impl == nullptr) {
        QKxMessageBox::information(this, tr("Seperation information"), tr("Current tab is not a text terminal tab."));
        return;
    }
    QList<QPointer<QWoTermWidget> > all = impl->termAll();
    if(all.length() <= 1) {
        QKxMessageBox::information(this, tr("Seperation information"), tr("Current tab has only one window, not need to seperate."));
        return;
    }
    int tabTotal = all.length() + m_tab->count() - 1;
    if(tabTotal >= QKxVer::instance()->maxTabCount()) {
        QKxMessageBox::information(this, tr("Version restrictions"), tr("The remaining number of tabs is not enough to seperate."));
        return;
    }
    QString targetName = impl->targetName();
    auto it = std::find_if(all.begin(), all.end(), [=](const QWoTermWidget* term) {
        return term->target() == targetName;
    });
    if(it != all.end()) {
        all.erase(it);
    }

    for(int i = all.length() - 1; i >= 0; i--) {
        QWoTermWidget *term = all.at(i);
        if(term == nullptr) {
            continue;
        }
        term->detachWidget();
        if(term->isLocalShell()) {
            attachLocalShell(term, idx+1);
        }else{
            const HostInfo& hi = QWoSshConf::instance()->find(term->target());
            if(hi.type == SshWithSftp) {
                attachSsh(term, idx+1);
            }else if(hi.type == Telnet) {
                attachTelnet(term, idx+1);
            }else if(hi.type == RLogin) {
                attachRLogin(term, idx+1);
            }
        }
    }
    if(impl->termCount() == 0) {
        impl->deleteLater();
    }
    m_tab->setCurrentIndex(idx);
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

void QWoShower::createTab(QWoShowerWidget *impl, const QIcon&icon, const QString& tabName, int pos)
{
    addWidget(impl);
    int idx = m_tab->insertTab(pos, icon, tabName);
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
        if(impl->isRemoteSession()) {
            menu.addAction(tr("New same session"), this, SLOT(onNewTheSameSession()));
            menu.addAction(tr("Copy host address"), this, SLOT(onCopyTabSessionAddress()));
        }
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
            menu.addAction(QIcon("../private/skins/black/nodes.png"), tr("Open remote session"), this, SLOT(onOpenRemoteSession()));
            menu.addAction(QIcon("../private/skins/black/console.png"), tr("Open local session"), this, SLOT(onOpenLocalSession()));
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
