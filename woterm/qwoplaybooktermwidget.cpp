/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwoplaybooktermwidget.h"
#include "qkxtermitem.h"
#include "qwosetting.h"
#include "qwoutils.h"
#include "qwosshconf.h"
#include "qkxmessagebox.h"
#include "qkxtouchpoint.h"

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
#include <QShowEvent>
#include <QFontDatabase>


QWoPlaybookTermWidget::QWoPlaybookTermWidget(QWidget *parent)
    : QKxTermWidget(parent)
{
    static int idx = 0;
    setObjectName(QString("QWoPlaybookTermWidget:%1").arg(idx++));
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    initDefault();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_term->showTermName(false);

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

    // only for android.
    m_term->setBlinkAlway(true);
}

QWoPlaybookTermWidget::~QWoPlaybookTermWidget()
{

}

void QWoPlaybookTermWidget::append(const QString &_log)
{
    QString log = _log;
    if(!log.endsWith("\r\n")) {
        log.append("\r\n");
    }
    m_term->parse(log.toUtf8());
}

void QWoPlaybookTermWidget::scrollToEnd()
{
    m_term->scrollToEnd();
}

void QWoPlaybookTermWidget::clear()
{
    m_term->clearAll();
}

void QWoPlaybookTermWidget::showEvent(QShowEvent *event)
{
    QKxTermWidget::showEvent(event);
#if defined (Q_OS_ANDROID)
#elif defined (Q_OS_IOS)
#else
    QWidget *parent = parentWidget();
    if(parent != nullptr) {
        QRect rt = parent->rect();
        setGeometry(rt);
    }
#endif
}

void QWoPlaybookTermWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose(event);
    QKxTermWidget::closeEvent(event);
}

void QWoPlaybookTermWidget::initDefault()
{
    QVariantMap mdata = QWoSetting::ttyDefault();
    resetProperty(mdata);
}

void QWoPlaybookTermWidget::onResetTermSize()
{
    m_term->updateTermSize();
}

void QWoPlaybookTermWidget::resetProperty(QVariantMap mdata)
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

    QFont ft = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString fontName = mdata.value("fontName", ft.family()).toString();
    int fontSize = mdata.value("fontSize", ft.pointSize()).toInt();

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
}
