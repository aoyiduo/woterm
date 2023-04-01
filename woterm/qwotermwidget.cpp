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


QWoTermWidget::QWoTermWidget(const QString& target, int gid, ETermType ttype, QWidget *parent)
    : QKxTermWidget(parent)
    , m_target(target)
    , m_gid(gid)
    , m_bexit(false)
    , m_ttype(ttype)
{
    static int idx = 0;
    setObjectName(QString("QWoTermWidget:%1").arg(idx++));
    addToTermImpl();
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    initDefault();
    initCustom();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_loading = new QWoLoadingWidget(QColor("#1296DB"), this);

    QString val = QWoSetting::value("property/shortcut").toString();
    QVariantMap mdata = QWoUtils::qBase64ToVariant(val).toMap();
    m_term->bindShortCut(QKxTermItem::SCK_Copy, mdata.value("SCK_Copy", m_term->defaultShortCutKey(QKxTermItem::SCK_Copy)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_Paste, mdata.value("SCK_Paste", m_term->defaultShortCutKey(QKxTermItem::SCK_Paste)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectAll, mdata.value("SCK_SelectAll", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectAll)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectLeft, mdata.value("SCK_SelectLeft", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectLeft)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectRight, mdata.value("SCK_SelectRight", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectRight)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectUp, mdata.value("SCK_SelectUp", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectUp)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectDown, mdata.value("SCK_SelectDown", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectDown)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectHome, mdata.value("SCK_SelectHome", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectHome)).value<QKeySequence>());
    m_term->bindShortCut(QKxTermItem::SCK_SelectEnd, mdata.value("SCK_SelectEnd", m_term->defaultShortCutKey(QKxTermItem::SCK_SelectEnd)).value<QKeySequence>()); 
}

QWoTermWidget::~QWoTermWidget()
{
    removeFromTermImpl();
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
    resetProperty(mdata);
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
        QVariantMap mdata = QWoUtils::qBase64ToVariant(hi.property).toMap();
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
    QString pathLast = QWoSetting::value("zmodem/lastPath").toString();
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
    QString path = file;
    int idx = path.lastIndexOf('/');
    if(idx > 0) {
        path = path.left(idx);
        QWoSetting::setValue("zmodem/lastPath", path);
    }
    m_historyFile = file;
}

void QWoTermWidget::onStopOutputHistoryFile()
{
    m_historyFile.clear();
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

void QWoTermWidget::resetProperty(QVariantMap mdata)
{
    if(mdata.isEmpty()) {
        return;
    }
    QString schema = mdata.value("colorSchema", DEFAULT_COLOR_SCHEMA).toString();
    m_term->setColorSchema(schema);

    QString keyboard = mdata.value("keyboard", DEFAULT_KEY_LAYOUT).toString();
    m_term->setKeyLayoutByName(keyboard);

    QString codec = mdata.value("textcodec", DEFAULT_TEXT_CODEC).toString();
    m_term->setTextCodec(codec);

    QString fontName = mdata.value("fontName", DEFAULT_FONT_FAMILY).toString();
    int fontSize = mdata.value("fontSize", DEFAULT_FONT_SIZE).toInt();
    m_term->setTerminalFont(fontName, fontSize);
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
    m_term->setDragCopyAndPaste(dragPaste);
    m_rkeyPaste = mdata.value("rkeyPaste", false).toBool();
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
