/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qwotermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwoutils.h"
#include "qwoglobal.h"
#include "qwotermmask.h"
#include "qwotermwidgetimpl.h"
#include "qwopasswordinput.h"
#include "qwoevent.h"
#include "qwoloadingwidget.h"
#include "qwofloatwindow.h"
#include "qwomainwindow.h"
#include "qwoshowerwidget.h"
#include "qwoshower.h"
#include "qkxmessagebox.h"
#include "qwoptytermwidget.h"
#include "qkxbackgroundimagerender.h"
#include "qkxutils.h"
#include "qwoshowerwidget.h"
#include "qkxtermitem.h"

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QClipboard>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QShortcut>
#include <QFileDialog>
#include <QFile>
#include <QFontDatabase>


QList<QPointer<QWoTermWidget>> QWoTermWidget::m_gsTermsAll;
QPointer<QWoTermWidget> QWoTermWidget::m_gsTermWidget;
QPointer<QKxBackgroundImageRender> QWoTermWidget::m_bkImageRender;

QWoTermWidget::QWoTermWidget(const QString& target, int gid, ETermType ttype, QWidget *parent)
    : QKxTermWidget(parent)
    , m_target(target)
    , m_gid(gid)
    , m_bexit(false)
    , m_rkeyMode(ERKM_NotDefined)
    , m_selectCopy(false)
    , m_ttype(ttype)
{
    static int idx = 0;
    addToTermImpl();
    setObjectName(QString("QWoTermWidget:%1").arg(idx++));    
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    initDefault();
    initCustom();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_loading = new QWoLoadingWidget(QColor("#1296DB"), this);

    m_term->installEventFilter(this);
    m_gsTermsAll.append(this);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(onTimeoutToSelectCopy()));
    QObject::connect((QKxTermItem*)m_term, &QKxTermItem::selectChanged, this, [=]{
        if(!m_selectCopy) {
            return;
        }
        timer->start(300);
    });
}

QWoTermWidget::~QWoTermWidget()
{
    m_gsTermsAll.removeAll(this);
    removeFromTermImpl();
}


QWoTermWidget *QWoTermWidget::lastFocusTermWidget()
{
    return m_gsTermWidget;
}

QList<QPointer<QWoTermWidget> > QWoTermWidget::termsAll()
{
    return m_gsTermsAll;;
}

void QWoTermWidget::setImplementWidget(QWoShowerWidget *impl)
{
    m_implementWidget = impl;
}

QWoShowerWidget *QWoTermWidget::implementWidget()
{
    return m_implementWidget;
}

void QWoTermWidget::closeAndDelete()
{
    m_bexit = true;
    deleteLater();
    QSplitter *splitParent = qobject_cast<QSplitter*>(parent());
    if(splitParent == nullptr) {
        return;
    }
    int cnt = splitParent->count();
    if(cnt == 1) {
        splitParent->deleteLater();
        splitParent = qobject_cast<QSplitter*>(splitParent->parent());
        while(splitParent != nullptr) {
            if(splitParent->count() > 1) {
                break;
            }
            splitParent->deleteLater();
            splitParent = qobject_cast<QSplitter*>(splitParent->parent());
        }
    }
}

void QWoTermWidget::triggerPropertyCheck()
{
    initDefault();
    initCustom();
}

QString QWoTermWidget::target() const
{
    return m_target;
}

int QWoTermWidget::gid() const
{
    return m_gid;
}

bool QWoTermWidget::isLocalShell()
{
    return m_ttype == ETTLocalShell;
}

void QWoTermWidget::showLoading(bool on)
{
    if(m_loading) {
        m_loading->setVisible(on);
    }
}

void QWoTermWidget::reloadProperty()
{
    initCustom();
}

bool QWoTermWidget::hasHistoryFile() const
{
    return m_term->hasHistoryFile();
}

void QWoTermWidget::outputHistoryToFile()
{
    onOutputHistoryToFile();
}

void QWoTermWidget::stopOutputHistoryFile()
{
    onStopOutputHistoryFile();
}

void QWoTermWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    QKxTermWidget::closeEvent(event);
}

void QWoTermWidget::resizeEvent(QResizeEvent *event)
{    
    QKxTermWidget::resizeEvent(event);
    m_loading->setGeometry(0, 0, width(), height());
}

bool QWoTermWidget::event(QEvent *ev)
{
    if(ev->type() == QWoEvent::EventType) {
        return handleWoEvent(ev);
    }
    return QKxTermWidget::event(ev);
}

void QWoTermWidget::initDefault()
{
    QVariantMap mdata = QWoSetting::ttyDefault();
    resetProperty(mdata, true);
}

void QWoTermWidget::initCustom()
{
    if(m_ttype == ETTLocalShell) {
        QVariantMap mdata = QWoSetting::localShell();
        resetProperty(mdata);
    }else if(m_ttype == ETTSerialPort) {
        QVariantMap mdata = QWoSetting::serialPort();
        resetProperty(mdata);
    }else{
        HostInfo hi = QWoSshConf::instance()->find(m_target);
        QVariantMap mdata = hi.merge(hi.type, QWoSetting::ttyDefault());
        resetProperty(mdata);
    }
}

void QWoTermWidget::onResetTermSize()
{
    m_term->updateTermSize();
}

void QWoTermWidget::onCleanHistory()
{
    m_term->cleanHistory();
}

void QWoTermWidget::onOutputHistoryToFile()
{
    QString pathLast = QWoSetting::value("histroy/lastSavePath").toString();
    QString file = QFileDialog::getSaveFileName(this, tr("Save history to file"), pathLast, tr("log (*.log)"));
    if(file.isEmpty()) {
        return;
    }
    QFile hit(file);
    if(!hit.open(QFile::WriteOnly)) {
        QKxMessageBox::warning(this, tr("Warning"), tr("Failed to create file."));
        return;
    }
    hit.close();
    m_term->setHistoryFile(file);
    QWoSetting::setValue("histroy/lastSavePath", file);
}

void QWoTermWidget::onStopOutputHistoryFile()
{
    m_term->stopHistoryFile();
}

void QWoTermWidget::onFloatThisTab()
{
    QWoFloatWindow *wfloat = qobject_cast<QWoFloatWindow*>(topLevelWidget());
    if(wfloat != nullptr) {
        return;
    }
    QWidget *parent = parentWidget();
    QWoShowerWidget *shower = qobject_cast<QWoShowerWidget*>(parent);
    while(parent != nullptr && shower == nullptr){
        parent = parent->parentWidget();
        shower = qobject_cast<QWoShowerWidget*>(parent);
    }
    if(shower != nullptr) {
        QMetaObject::invokeMethod(QWoMainWindow::shower(), "floatSession", Q_ARG(QWoShowerWidget*, shower), Q_ARG(bool, false));
    }
}

void QWoTermWidget::onTimeoutToSelectCopy()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if(timer) {
        timer->stop();
    }
    if(!m_selectCopy) {
        return;
    }
    QString txt = m_term->selectedText();
    if(txt.isEmpty()) {
        return;
    }
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText(txt);
}

void QWoTermWidget::resetProperty(QVariantMap mdata, bool force)
{
    if(mdata.isEmpty() && !force) {
        return;
    }
    QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
    m_term->setColorSchema(schema);

    QString keyboard = mdata.value("keyTranslator", DEFAULT_KEY_TRANSLATOR).toString();
    m_term->setKeyTranslatorByName(keyboard);

    QString codec = mdata.value("textCodec", DEFAULT_TEXT_CODEC).toString();
    m_term->setTextCodec(codec);

    QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString fontName = mdata.value("fontName", ft.family()).toString();
    int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();
    QStringList families = QKxUtils::availableFontFamilies();
    if(families.contains(fontName)) {
        m_term->setTerminalFont(fontName, fontSize);
    }
    QString cursorType = mdata.value("cursorType", "block").toString();
    if(cursorType.isEmpty() || cursorType == "block") {
        m_term->setCursorType(QKxTermItem::CT_Block);
    }else if(cursorType == "underline") {
        m_term->setCursorType(QKxTermItem::CT_Underline);
    }else {
        m_term->setCursorType(QKxTermItem::CT_IBeam);
    }
    int lines = mdata.value("historyLength", DEFAULT_HISTORY_LINE_LENGTH).toInt();
    m_term->setHistorySize(lines);
    bool dragPaste = mdata.value("dragPaste", false).toBool();
    bool dragInput = mdata.value("dragInput", false).toBool();
    if(dragPaste) {
        m_term->setDragTextMode(QKxTermItem::DTM_DragCopyAndPaste);
    }else if(dragInput) {
        m_term->setDragTextMode(QKxTermItem::DTM_DragToInput);
    }else{
        m_term->setDragTextMode(QKxTermItem::DTM_NotDefined);
    }
    bool rkeyCopy = mdata.value("rkeyCopy", false).toBool();
    bool rkeyPaste = mdata.value("rkeyPaste", false).toBool();
    if(rkeyCopy) {
        m_rkeyMode = ERKM_Copy;
    }else if(rkeyPaste) {
        m_rkeyMode = ERKM_CopyPaste;
    }else{
        m_rkeyMode = ERKM_NotDefined;
    }
    m_selectCopy = mdata.value("selectCopy", false).toBool();


    QString path = QWoSetting::terminalBackgroundImage();
    int alpha = QWoSetting::terminalBackgroundImageAlpha();
    bool smooth = QWoSetting::terminalBackgroundImageEdgeSmooth();
    QString position = QWoSetting::terminalBackgroundImagePosition();
    if(path.isEmpty()) {
        if(m_bkImageRender) {
            m_bkImageRender->deleteLater();
        }
        return;
    }
    if(m_bkImageRender == nullptr) {
        m_bkImageRender = new QKxBackgroundImageRender(QCoreApplication::instance());
    }
    m_bkImageRender->setImagePath(path, position, smooth, alpha);
    m_term->setBackgroundImageRender(m_bkImageRender);
}

void QWoTermWidget::splitWidget(const QString& target, int gid, bool vertical)
{
    QSplitter *splitParent = qobject_cast<QSplitter*>(parent());
    if(splitParent == nullptr) {
        return;
    }

    QWoTermWidgetImpl *impl = findTermImpl();
    impl->updateEnable(false);
    int cnt = splitParent->count();
    QSplitter *splitter = splitParent;
    if(cnt > 1) {
        QList<int> ls = splitParent->sizes();
        int idx = splitParent->indexOf(this);
        QSplitter *splitNew = new QSplitter(splitParent);
        splitParent->insertWidget(idx, splitNew);
        splitNew->setChildrenCollapsible(false);
        splitNew->addWidget(this);
        splitParent->setSizes(ls);
        splitter = splitNew;
        splitter->setHandleWidth(1);
        splitter->setOpaqueResize(false);
    }
    splitter->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    QWoTermWidget *term = impl->createTermWidget(target, gid, splitter);
    splitter->addWidget(term);
    term->show();
    int width = splitter->width();
    QList<int> ls;
    ls << width / 2 << width / 2;
    splitter->setSizes(ls);
    impl->updateEnable(true);
    impl->addToList(term);
}

bool QWoTermWidget::attachWidget(QWoTermWidget *term, bool vertical)
{
    QSplitter *splitParent = qobject_cast<QSplitter*>(parent());
    if(splitParent == nullptr) {
        return false;
    }

    QWoTermWidgetImpl *impl = findTermImpl();
    impl->updateEnable(false);
    int cnt = splitParent->count();
    QSplitter *splitter = splitParent;
    if(cnt > 1) {
        QList<int> ls = splitParent->sizes();
        int idx = splitParent->indexOf(this);
        QSplitter *splitNew = new QSplitter(splitParent);
        splitParent->insertWidget(idx, splitNew);
        splitNew->setChildrenCollapsible(false);
        splitNew->addWidget(this);
        splitParent->setSizes(ls);
        splitter = splitNew;
        splitter->setHandleWidth(1);
        splitter->setOpaqueResize(false);
    }
    splitter->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    term->setParent(splitter);
    splitter->addWidget(term);
    QObject *obj = term->parent();
    QWidget *obj2 = term->parentWidget();
    Q_ASSERT(obj == splitter && obj2 == splitter);
    term->show();
    int width = splitter->width();
    QList<int> ls;
    ls << width / 2 << width / 2;
    splitter->setSizes(ls);
    impl->updateEnable(true);
    impl->addToList(term);
    return true;
}

void QWoTermWidget::detachWidget()
{
    m_bexit = true;
    removeFromTermImpl();
    QSplitter *splitParent = qobject_cast<QSplitter*>(parent());
    if(splitParent == nullptr) {
        return;
    }
    int cnt = splitParent->count();
    if(cnt == 1) {
        splitParent->deleteLater();
        splitParent = qobject_cast<QSplitter*>(splitParent->parent());
        while(splitParent != nullptr) {
            if(splitParent->count() > 1) {
                break;
            }
            splitParent->deleteLater();
            splitParent = qobject_cast<QSplitter*>(splitParent->parent());
        }
    }
    setParent(nullptr);
}

QWoTermWidget::ETermType QWoTermWidget::termType() const
{
    return m_ttype;
}

QWoTermWidgetImpl *QWoTermWidget::findTermImpl()
{
    QWidget *widgetParent = parentWidget();
    QWoTermWidgetImpl *impl = qobject_cast<QWoTermWidgetImpl*>(widgetParent);
    while(impl == nullptr) {
        widgetParent = widgetParent->parentWidget();
        if(widgetParent == nullptr) {
            return nullptr;
        }
        impl = qobject_cast<QWoTermWidgetImpl*>(widgetParent);
    }
    return impl;
}

void QWoTermWidget::addToTermImpl()
{
    QWoTermWidgetImpl *impl = findTermImpl();
    if(impl) {
        impl->addToList(this);
    }
}

void QWoTermWidget::removeFromTermImpl()
{
    QWoTermWidgetImpl *impl = findTermImpl();
    if(impl) {
        impl->removeFromList(this);
    }
}

void QWoTermWidget::onBroadcastMessage(int type, QVariant msg)
{
    qDebug() << "type" << type << "msg" << msg;
}

bool QWoTermWidget::handleWoEvent(QEvent *ev)
{
    if(ev->type() != QWoEvent::EventType) {
        return false;
    }
    QWoEvent *we = (QWoEvent*)ev;
    QWoEvent::WoEventType type = we->eventType();
    if(type == QWoEvent::PropertyChanged) {
        triggerPropertyCheck();
    }

    return false;
}

bool QWoTermWidget::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type type = ev->type();
    if(type == QEvent::FocusIn) {
        if(obj == m_term) {
            m_gsTermWidget = this;
            if(m_implementWidget) {
                m_implementWidget->setLastFocusWidget(this);
            }
        }
    }
    return QKxTermWidget::eventFilter(obj, ev);
}
