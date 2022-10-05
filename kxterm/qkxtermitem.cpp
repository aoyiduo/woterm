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

#include "qkxtermitem.h"
#include "qvteimpl.h"
#include "qkxhistory.h"
#include "qkxscreen.h"
#include "qkxview.h"
#include "qkxutils.h"
#include "qkxhistory.h"
#include "qkxkeytranslator.h"
#include "qkxcolorschema.h"
#include "qkxechoinput.h"

#include <stdlib.h>
#include <stdio.h>

#include <QGuiApplication>
#include <QClipboard>
#include <QPainter>
#include <QTimer>
#include <QtMath>
#include <QDebug>
#include <QFocusEvent>
#include <QPainter>
#include <QLabel>

#define REPAINT_TIMEOUT1 (20)
#define REPAINT_TIMEOUT2 (40)
#define BLINK_MAX_COUNT (9)

QKxTermItem::QKxTermItem(QWidget* parent)
    : QWidget(parent)
    , m_rows(40)
    , m_columns(80)
    , m_spaceLine(0)
    , m_lineWidth(0)
    , m_scrollValue(-1)
    , m_scrollMaxValue(-1)
    , m_autoScrollToEnd(true)    
    , m_highContrast(false)
    , m_focus(false)    
    , m_tripleClick(false)
    , m_blinkAlway(false)
    , m_bEchoInputEnabled(false)
{
    static int idx = 0;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setObjectName(QString("QKxTermItem:%1").arg(idx++));
    setInputEnable(true);

    m_keyCopy = QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_C);
    m_keyPaste = QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_V);
    m_keyUpSelect = QKeySequence(Qt::SHIFT + Qt::Key_Up);
    m_keyDownSelect = QKeySequence(Qt::SHIFT + Qt::Key_Down);
    m_keyLeftSelect = QKeySequence(Qt::SHIFT + Qt::Key_Left);
    m_keyRightSelect = QKeySequence(Qt::SHIFT + Qt::Key_Right);
    m_keyHomeSelect = QKeySequence(Qt::SHIFT + Qt::Key_Home);
    m_keyEndSelect = QKeySequence(Qt::SHIFT + Qt::Key_End);
    m_keySelectAll = QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_A);

    initTitle();

    m_vte = new QVteImpl(this);
    m_view = new QKxView(this);
    m_view->setScreen(m_vte->screen());
    QObject::connect(m_vte, SIGNAL(contentChanged(bool)), this, SLOT(onContentChanged(bool)));
    QObject::connect(m_view, SIGNAL(selectChanged()), this, SLOT(onSelectChanged()));
    QObject::connect(m_vte, SIGNAL(screenChanged()), this, SLOT(onScreenChanged()));
    QObject::connect(m_vte, SIGNAL(sendData(const QByteArray&)), this, SIGNAL(sendData(const QByteArray&)));
    QObject::connect(m_vte, SIGNAL(titleChanged(const QString&)), this, SIGNAL(titleChanged(const QString&)));

    m_blinker = new QTimer();
    QObject::connect(m_blinker, SIGNAL(timeout()), this, SLOT(onBlinkTimeout()));
    m_blinker->start(500);

    setCursorType(CT_Block);
    setMouseCursor(Qt::IBeamCursor);
    setSelectionMode(SM_LineText);

    m_ticker = new QTimer(this);
    m_ticker2 = new QTimer(this);
    m_ticker->setSingleShot(true);
    m_ticker2->setSingleShot(true);
    QObject::connect(m_ticker, SIGNAL(timeout()), this, SLOT(onRepaintTimeout()));
    QObject::connect(m_ticker2, SIGNAL(timeout()), this, SLOT(onRepaintTimeout()));

    setKeyLayoutByName(DEFAULT_KEY_LAYOUT);
    setColorSchema(DEFAULT_COLOR_SCHEMA);
    QFont font;
    QString suggest = QKxUtils::suggestFamily();
    font.setFamily(suggest);
    font.setPointSize(DEFAULT_FONT_SIZE);
    font.setWeight(QFont::Normal);
    //font.setStyleHint(QFont::TypeWriter);
    //font.setStyleStrategy(QFont::StyleStrategy(QFont::ForceIntegerMetrics));
    font.setStyleName(QString());
    font.setStyleHint(QFont::Monospace);
    font.setStyleStrategy(QFont::StyleStrategy(QFont::ForceIntegerMetrics|QFont::PreferAntialias));
    font.setFixedPitch(true);
    font.setKerning(false);
    setFont(font);
}

QFont QKxTermItem::font() const
{
    return m_font;
}

void QKxTermItem::setFont(const QFont &ft)
{
    if(ft != m_font) {
        m_font = ft;        
        updateFontInfo();
        updateTermSize();
        updateView(PF_FullScreen);
        m_vte->setFont(ft);
    }
}

QString QKxTermItem::textCodec() const
{
    return m_vte->textCodec();
}

void QKxTermItem::setTextCodec(const QString &c)
{
    m_vte->setTextCodec(c);
}

bool QKxTermItem::inputEnable() const
{
    return true;
}

void QKxTermItem::setInputEnable(bool on)
{

}

int QKxTermItem::fontSize() const
{
    return m_font.pointSize();
}

void QKxTermItem::setFontSize(int sz)
{
    QFont ft = m_font;
    ft.setPointSize(sz);
    setFont(ft);
}

QString QKxTermItem::fontFamily() const
{
    return m_font.family();
}

void QKxTermItem::setFontFamily(const QString &name)
{
    QFont ft = m_font;
    ft.setFamily(name);
    setFont(ft);
}

bool QKxTermItem::blinkAlway() const
{
    return m_blinkAlway;
}

void QKxTermItem::setBlinkAlway(bool on)
{
    m_blinkAlway = on;
}

int QKxTermItem::scrollMaxValue() const
{
    return m_view->historyLineCount();
}

int QKxTermItem::scrollValue() const
{
    return m_scrollValue;
}

bool QKxTermItem::highContrast() const
{
    return m_highContrast;
}

void QKxTermItem::setHighContrast(bool on)
{
    m_highContrast = on;
}

void QKxTermItem::setBackgroundColor(const QColor &clr)
{
    if(m_backgroundColor != clr) {
        m_backgroundColor = clr;
        m_flagPaints |= PF_FullScreen;
        emit backgroundChanged(clr);
    }
}

QColor QKxTermItem::backgroundColor() const
{
    return m_backgroundColor;
}

QKxTermItem::CursorType QKxTermItem::cursorType() const
{
    return m_cursorType;
}

void QKxTermItem::setCursorType(const QKxTermItem::CursorType &t)
{
    m_cursorType = t;
}

QString QKxTermItem::keyTable() const
{
    return m_keyTranslator->path();
}

QCursor QKxTermItem::mouseCursor() const
{
    return cursor();
}

void QKxTermItem::setMouseCursor(const QCursor &cur)
{
    setCursor(cur);
}

SelectionMode QKxTermItem::selectionMode() const
{
    return m_selectMode;
}

void QKxTermItem::setSelectionMode(SelectionMode m)
{
    m_selectMode = m;
}

QSize QKxTermItem::termSize() const
{
    return QSize(m_columns, m_rows);
}

QSize QKxTermItem::termViewSize() const
{
    return QSize(m_columns, m_view->historyLineCount() + m_rows);
}

QString QKxTermItem::selectedText() const
{
    return m_view->selectedText();
}

bool QKxTermItem::echoInputEnabled() const
{
    return m_bEchoInputEnabled;
}

void QKxTermItem::setEchoInputEnabled(bool on)
{
    m_bEchoInputEnabled = on;
}

void QKxTermItem::setEchoInput(QKxEchoInput *echo)
{
    m_echoInput = echo;
}

void QKxTermItem::scrollToEnd()
{
    int value = m_view->historyLineCount();
    if(value != m_scrollValue) {
        updateScrollValue(value);
    }
}

bool QKxTermItem::scrollTo(int y)
{
    if(y != m_scrollValue) {
        updateScrollValue(y);
        return true;
    }
    return false;
}

QKxKeyTranslator* QKxTermItem::keyLayout() const
{
    return m_keyTranslator;
}

void QKxTermItem::setKeyLayoutByName(const QString &name)
{
    QKxKeyTranslator *translator = QKxUtils::keyboardLayout(name);
    setKeyLayout(translator);
}

void QKxTermItem::setKeyLayout(QKxKeyTranslator *translator)
{
    m_keyTranslator = translator;
}

QString QKxTermItem::keyLayoutName() const
{
    return m_keyTranslator->name();
}

QString QKxTermItem::colorSchema() const
{
    return m_schema;
}

void QKxTermItem::setColorSchema(const QString &name)
{
    QString path = QKxUtils::colorSchemaPath(name);
    if(!path.isEmpty()) {
        m_schema = name;
        if(loadColorSchema(path)) {
            setBackgroundColor(m_colorSchema->background());
        }
    }
}

QString QKxTermItem::termName() const
{
    return m_title->text();
}

void QKxTermItem::setTermName(const QString &name)
{
    m_title->setText(name);
    resetTitlePosition(false);
}

QPoint QKxTermItem::cursorToScreenPosition()
{
    return m_view->cursorToScreenPosition();
}

QPoint QKxTermItem::cursorToViewPosition()
{
    return m_view->cursorToViewPosition();
}

QPoint QKxTermItem::viewToScreenPosition(const QPoint &pt)
{
    return m_view->viewToScreenPosition(pt);
}

int QKxTermItem::historySize() const
{
    return m_vte->historySize();
}

void QKxTermItem::setHistorySize(int s)
{
    m_vte->setHistorySize(s);
}

void QKxTermItem::setHistoryFile(const QString &file)
{
    m_vte->setHistoryFile(file);
}

void QKxTermItem::stopHistoryFile()
{
    m_vte->stopHistoryFile();
}

void QKxTermItem::cleanHistory()
{
    m_vte->cleanHistory();
}

bool QKxTermItem::appMode()
{
    return m_vte->appMode();
}

QString QKxTermItem::plainText(const QPoint &start, const QPoint &end)
{
    return m_view->plainText(start, end);
}

bool QKxTermItem::trapCommand(const QString &cmd, QString &content, int &code, int timeout)
{
    waitInput();
    QByteArray _cmd = cmd.simplified().toUtf8();
    if(_cmd.at(_cmd.length() - 1) != '\r') {
        _cmd.append('\r');
    }
    emit sendData(_cmd);

    QPoint pt1 = m_view->cursorToViewPosition();
    int id = m_view->setCapture(pt1);
    wait(timeout);
    if(!m_view->releaseCapture(id, &pt1)) {
        return false;
    }
    QPoint pt2 = m_view->cursorToViewPosition();
    content = plainText(pt1, pt2);
    code = lastExitCode();
    return true;
}

void QKxTermItem::waitInput()
{
    emit sendData("\r");
    wait();
}

void QKxTermItem::bindShortCut(QKxTermItem::ShortCutKey sck, QKeySequence key)
{
    switch(sck)
    {
    case SCK_Copy:
        m_keyCopy = key;
        break;
    case SCK_Paste:
        m_keyPaste = key;
        break;
    case SCK_SelectAll:
        m_keySelectAll = key;
        break;
    case SCK_SelectDown:
        m_keyDownSelect = key;
        break;
    case SCK_SelectEnd:
        m_keyEndSelect = key;
        break;
    case SCK_SelectHome:
        m_keyHomeSelect = key;
        break;
    case SCK_SelectLeft:
        m_keyLeftSelect = key;
        break;
    case SCK_SelectRight:
        m_keyRightSelect = key;
        break;
    case SCK_SelectUp:
        m_keyUpSelect = key;
        break;
    }
}


QKeySequence QKxTermItem::defaultShortCutKey(QKxTermItem::ShortCutKey sck)
{
    switch(sck)
    {
    case SCK_Copy:
        return QKeySequence(Qt::ALT + Qt::Key_C);
    case SCK_Paste:
        return QKeySequence(Qt::ALT + Qt::Key_V);
    case SCK_SelectAll:
        return QKeySequence(Qt::ALT + Qt::Key_A);
    case SCK_SelectDown:
        return QKeySequence(Qt::SHIFT + Qt::Key_Down);
    case SCK_SelectEnd:
        return QKeySequence(Qt::SHIFT + Qt::Key_End);
    case SCK_SelectHome:
        return QKeySequence(Qt::SHIFT + Qt::Key_Home);
    case SCK_SelectLeft:
        return QKeySequence(Qt::SHIFT + Qt::Key_Left);
    case SCK_SelectRight:
        return QKeySequence(Qt::SHIFT + Qt::Key_Right);
    case SCK_SelectUp:
        return QKeySequence(Qt::SHIFT + Qt::Key_Up);
    }
    return QKeySequence();
}

QKeySequence QKxTermItem::currentShortCutKey(QKxTermItem::ShortCutKey sck)
{
    switch(sck)
    {
    case SCK_Copy:
        return m_keyCopy;
    case SCK_Paste:
        return m_keyPaste;
    case SCK_SelectAll:
        return m_keySelectAll;
    case SCK_SelectDown:
        return m_keyDownSelect;
    case SCK_SelectEnd:
        return m_keyEndSelect;
    case SCK_SelectHome:
        return m_keyHomeSelect;
    case SCK_SelectLeft:
        return m_keyLeftSelect;
    case SCK_SelectRight:
        return m_keyRightSelect;
    case SCK_SelectUp:
        return m_keyUpSelect;
    }
    return QKeySequence();
}

QStringList QKxTermItem::availableKeyLayouts() const
{
    return QKxUtils::availableKeyLayouts();
}

QStringList QKxTermItem::availableColorSchemas() const
{
    return QKxUtils::availableColorSchemas();
}

QStringList QKxTermItem::availableFontFamilies() const
{
    return QKxUtils::availableFontFamilies();
}

void QKxTermItem::clearAll()
{
    m_vte->clearAll();
    clearSelection();
    updateView(PF_FullScreen);
}

void QKxTermItem::preview()
{
    QByteArray seqTxt;
    seqTxt.append("\033[31mRed \033[32mGreen \033[33mYellow \033[34mBlue");
    seqTxt.append("\r\n\033[35mMagenta \033[36mCyan \033[37mWhite \033[39mDefault");
    seqTxt.append("\r\n\033[40mBlack \033[41mRed \033[42mGreen \033[43mYellow \033[44mBlue");
    seqTxt.append("\r\n\033[45mMagenta \033[46mCyan \033[47mWhite \033[49mDefault");
    parse(seqTxt);
}

void QKxTermItem::tryToCopy()
{
    QClipboard *clip = QGuiApplication::clipboard();
    QString selTxt = selectedText();
    if(!selTxt.isEmpty()) {
        clip->setText(selTxt);
    }
}

void QKxTermItem::tryToPaste()
{
    QClipboard *clip = QGuiApplication::clipboard();
    QString clipTxt = clip->text();

    if(!clipTxt.isEmpty()){
        if(m_echoInput != nullptr && m_bEchoInputEnabled) {
            m_echoInput->tryToPaste(clipTxt);
        }else{
            QByteArray buf = clipTxt.toUtf8();
            emit sendData(buf);
        }
    }
}

void QKxTermItem::resetState()
{
    if(appMode()){
        parse("\033[?25h\033[?25l\033[?1049l");        
    }
    m_vte->resetState();
    updateView(PF_FullScreen);
}

void QKxTermItem::echoInput(const QByteArray &data)
{
    m_vte->process(data);
}

int QKxTermItem::wait(int timeout)
{
    QEventLoop loop;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&loop] () {
        loop.exit(-1);
    });
    QObject::connect(m_vte, SIGNAL(titleChanged(const QString&)), &loop, SLOT(quit()));
    timer.setSingleShot(true);
    timer.setInterval(timeout);
    timer.start();
    int code = loop.exec();
    return code;
}

int QKxTermItem::lastExitCode()
{
    emit sendData("echo $?\r");
    QPoint pt1 = m_view->cursorToViewPosition();
    int id = m_view->setCapture(pt1);
    if(wait() != 0) {
        return -1;
    }
    if(!m_view->releaseCapture(id, &pt1)) {
        return -2;
    }
    QPoint pt2 = m_view->cursorToViewPosition();
    QStringList lines = plainText(pt1, pt2).split('\n');
    QString code = "";
    for(int i= 0; i < lines.count(); i++) {
        if(lines.at(i).simplified().endsWith("echo $?")) {
            if(i+1 < lines.count()) {
                code = lines.at(i+1).simplified();
                break;
            }
        }
    }
    if(code.isEmpty()) {
        return -3;
    }
    bool ok = false;
    int icode = code.toInt(&ok);
    if(!ok) {
        return -4;
    }
    return icode;
}

void QKxTermItem::initTitle()
{
    m_title = new QLabel(this);
    m_title->setAutoFillBackground(true);
    QPalette pal(Qt::gray);
    m_title->setPalette(pal);
    m_title->setContentsMargins(5, 2, 5, 2);
    m_title->setTextInteractionFlags(Qt::NoTextInteraction);
    m_title->installEventFilter(this);
}

void QKxTermItem::resetTitlePosition(bool bycursor)
{
    QSize sz = size();
    QSize sh = m_title->sizeHint();
    int y = 0;
    if(bycursor) {
        y = m_title->y() > sz.height() / 2 ? 0 :  sz.height()-sh.height();
    }else if(m_title->y() > 10){
        y = sz.height()-sh.height();
    }
    m_title->setGeometry(sz.width() - sh.width() - 1, y, sh.width(), sh.height());
}

void QKxTermItem::parse(const QByteArray &data)
{
    //qDebug() << data;
    if(m_echoInput != nullptr) {
        m_echoInput->updateCursor(true);
    }
    m_vte->process(data);
}

void QKxTermItem::parseError(const QByteArray &data)
{
    static const char redPenOn[] = "\033[1m\033[31m";
    static const int onLength = sizeof(redPenOn);
    static const char redPenOff[] = "\033[0m";
    static const int offLength = sizeof(redPenOff);
    QByteArray buf;
    buf.append(redPenOn, onLength);
    buf.append("\r\n");
    buf.append(data);
    buf.append("\r\n");
    buf.append(redPenOff, offLength);
    m_vte->process(buf);
}

void QKxTermItem::updateTermSize(int rows, int cols)
{
    m_vte->resize(rows, cols);
}

bool QKxTermItem::loadKeyLayout(const QString &path)
{
    if(m_keyTranslator == nullptr) {
        m_keyTranslator = new QKxKeyTranslator(this);
    }
    return m_keyTranslator->load(path);
}

bool QKxTermItem::loadColorSchema(const QString &path)
{
    if(m_colorSchema == nullptr) {
        m_colorSchema = new QKxColorSchema(this);
    }
    if(m_colorSchema->load(path)) {
       setBackgroundColor(m_colorSchema->background());
       updateView(PF_FullScreen);
       return true;
    }
    return false;
}

bool QKxTermItem::find(const QString &key, bool match, bool regular)
{
    if(m_view->find(key, match, regular)) {
        QMap<QPoint, QPoint> sel = m_view->selection();
        if(!sel.isEmpty()) {            
            int y = sel.firstKey().y();
            if(!isLineVisible(y)){
                scrollTo(y);
            }
            return true;
        }
    }
    return false;
}

bool QKxTermItem::findPrev(bool match, bool regular)
{
    if(m_view->findPrev(match, regular)) {
        QMap<QPoint, QPoint> sel = m_view->selection();
        if(!sel.isEmpty()) {
            int y = sel.firstKey().y();
            if(!isLineVisible(y)){
                scrollTo(y - m_rows + 1);
            }
            return true;
        }
    }
    return false;
}

bool QKxTermItem::findNext(bool match, bool regular)
{
    if(m_view->findNext(match, regular)) {
        QMap<QPoint, QPoint> sel = m_view->selection();
        if(!sel.isEmpty()) {
            int y = sel.firstKey().y();
            if(!isLineVisible(y)){
                scrollTo(y);
            }
            return true;
        }
    }
    return false;
}

void QKxTermItem::findAll(bool match, bool regular)
{
    m_view->findAll(match, regular);
}

void QKxTermItem::clearSelection()
{
    m_selectStart = m_selectEnd = QPoint(-1, -1);
    m_view->clearSelection();
}

QVariant QKxTermItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return inputMethodQuery(query, QVariant());
}

void QKxTermItem::updateScrollPosition(qreal position)
{
    qreal total = m_view->lineCount();
    int val = qCeil(total * position);
    updateScrollValue(val);
}

void QKxTermItem::onScreenChanged()
{
    m_view->setScreen(m_vte->screen());
    updateScrollValue(m_view->historyLineCount());
}

void QKxTermItem::onBlinkTimeout()
{
    // cursor
    if(!m_cursorRect.isEmpty()) {
        if(m_focus || m_blinkAlway) {
            m_cursorLeftBlink = BLINK_MAX_COUNT;
        }else{
            m_cursorLeftBlink--;
        }
        if(m_cursorLeftBlink > -2) {
            m_cursorReverse = !m_cursorReverse;
        }else{
            m_cursorReverse = false;
        }
        if(m_cursorLeftBlink > -3) {
            update(m_cursorRect);
        }
    }

    //blink
    if(!m_blinkRects.isEmpty()) {
        if(m_focus || m_blinkAlway) {
            m_blinkCountLeft = 10;
        }else{
            m_blinkCountLeft--;
        }
        if(m_blinkCountLeft > -2) {
            m_blinkShow = !m_blinkShow;
        }else{
            m_blinkShow = true;
        }
        if(m_blinkCountLeft > -3) {
            for(int i = 0; i < m_blinkRects.length(); i++) {
                update(m_blinkRects.at(i));
            }
        }
    }
}

void QKxTermItem::onSelectChanged()
{
    updateView(PF_Selection);
}

void QKxTermItem::onContentChanged(bool full)
{
    updateView(full ? PF_Normal : PF_FullScreen);
    if(m_autoScrollToEnd) {
        updateScrollValue(m_view->historyLineCount());
    }
}

void QKxTermItem::onRepaintTimeout()
{
    m_ticker->stop();
    m_ticker2->stop();
    if(m_flagPaints > 0) {
        //qDebug() << "paintCount" << m_paintCount;
        if(m_autoScrollToEnd) {
            QPoint pt = m_view->cursorToViewPosition();
            if(pt.y() > m_rows) {
                updateScrollValue(m_view->historyLineCount());
            }
        }
        updateImage();
        m_flagPaints = PF_None;
    }
}

void QKxTermItem::onTripleClickTimeout()
{
    m_tripleClick = false;
}

void QKxTermItem::onSelectClickTimeout()
{
    if(m_selectStart == m_selectEnd && m_selectEnd != QPoint(-1, -1)) {
        m_selectStart = m_selectEnd = QPoint(-1, -1);
    }
}

void QKxTermItem::paint(QPainter *p)
{
    if(m_image.isEmpty()) {
        return;
    }
    p->save();
    p->setFont(m_font);
    p->setLayoutDirection(Qt::LeftToRight);
    m_blinkRects.clear();
    m_cursorRect = QRect();
    for(int r = 0; r < m_rows && r < m_image.length(); r++) {
        drawLine(p, r, m_image.at(r));
    }
    if(!m_preeditText.isEmpty()){
        p->fillRect(m_preeditRect, m_colorSchema->foreground());
        p->setPen(m_colorSchema->background());
        p->drawText(m_preeditRect, Qt::AlignBaseline|Qt::TextDontClip, m_preeditText);
    }
    p->restore();
}

void QKxTermItem::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setClipRegion(ev->region());
    p.fillRect(rect(), m_backgroundColor);
    paint(&p);
}

void QKxTermItem::focusInEvent(QFocusEvent *ev)
{
    m_focus = true;
    ev->accept();
    m_ptClicked = QPoint(-1, -1);
}

void QKxTermItem::focusOutEvent(QFocusEvent *ev)
{
    m_focus = false;
    m_ptClicked = QPoint(-1, -1);
}

void QKxTermItem::resizeEvent(QResizeEvent *ev)
{
    updateTermSize();
    //qDebug() << objectName() << "geometryChanged" << newGeometry << oldGeometry;
    m_flagPaints |= PF_FullScreen;
    resetTitlePosition(false);
}

void QKxTermItem::keyPressEvent(QKeyEvent *ev)
{
    ev->accept();
    if(m_bEchoInputEnabled && m_echoInput != nullptr) {
        m_echoInput->onKeyPressEvent(ev);
        return;
    }
    int key = ev->key();
    Qt::KeyboardModifiers modifier = ev->modifiers();
    if(modifier & Qt::ShiftModifier) {
        key += Qt::SHIFT;
    }
    if(modifier & Qt::ControlModifier) {
        key += Qt::CTRL;
    }
    if(modifier & Qt::MetaModifier) {
        key += Qt::META;
    }
    if(modifier & Qt::AltModifier) {
        key += Qt::ALT;
    }
    QKeySequence seq(key);
    if(seq == m_keyCopy) {
        tryToCopy();
    }else if(seq == m_keyPaste) {
        tryToPaste();
    }else if(seq == m_keyUpSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        int y = m_selectEnd.y() - 1;
        m_selectEnd.setY(y > 0 ? y : 0);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keyDownSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        int y = m_selectEnd.y() + 1;
        m_selectEnd.setY(y >= m_view->lineCount() ? m_view->lineCount() - 1 : y);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keyLeftSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        int x = m_selectEnd.x() - 1;
        m_selectEnd.setX(x > 0 ? x : 0);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keyRightSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        int x = m_selectEnd.x() + 1;
        m_selectEnd.setX(x >= m_columns ? m_columns - 1 : x);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keyHomeSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        m_selectEnd.setX(0);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keyEndSelect){
        if(m_selectStart == QPoint(-1,-1)) {
            TermCursor tc = m_view->cursor();
            m_selectStart = m_selectEnd = QPoint(tc.x, tc.y + m_scrollValue);
        }
        m_selectEnd.setX(m_columns-1);
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else if(seq == m_keySelectAll){
        m_selectStart = QPoint(0, 0);
        m_selectEnd = QPoint(m_columns-1, m_view->lineCount());
        qDebug() << "start:" << m_selectStart << "end:" << m_selectEnd;
        m_view->setSelection(m_selectStart, m_selectEnd);
    }else{
        if(modifier == Qt::NoModifier){
            clearSelection();
        }
        handleKeyEvent(ev);
    }
}

void QKxTermItem::keyReleaseEvent(QKeyEvent *ev)
{
    ev->accept();
    if(m_bEchoInputEnabled && m_echoInput != nullptr) {
        m_echoInput->onKeyReleaseEvent(ev);
        return;
    }
}

void QKxTermItem::mousePressEvent(QMouseEvent *ev)
{
    ev->accept();
    setFocus();    
    if(ev->buttons() & Qt::LeftButton) {        
        if(m_tripleClick) {
            // only world selection can come here.
            QMap<QPoint, QPoint> sels = m_view->selection();
            if(sels.isEmpty()) {
                return;
            }
            QRect sel = QRect(sels.firstKey(), sels.first()).normalized();
            sel.translate(0, -m_scrollValue);
            QRect rt = QRect(sel.left() * m_fontWidth,
                             sel.top() * (m_fontHeight + m_spaceLine + m_lineWidth),
                             (sel.width()+1) * m_fontWidth,
                             (sel.height()+1) *(m_fontHeight + m_spaceLine + m_lineWidth));
            if(rt.contains(ev->pos())) {
                m_view->selectLine(sel.y() + m_scrollValue);
                return;
            }
        }
        Qt::KeyboardModifiers modifier = ev->modifiers();
        if(modifier & Qt::ShiftModifier) {
            if(m_selectEnd != m_selectStart) {
                QPoint pt = ev->pos();
                int l = pt.x() / m_fontWidth;
                int t = pt.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
                m_selectEnd = QPoint(l, t + m_scrollValue);
                m_view->setSelection(m_selectStart, m_selectEnd);
                return;
            }
        }
        clearSelection();
        m_ptClicked = ev->pos();
    }
}

void QKxTermItem::mouseMoveEvent(QMouseEvent *ev)
{
    if(ev->buttons() & Qt::LeftButton) {
        QPoint pt = ev->pos();
        if(m_ptClicked == QPoint(-1,-1)) {
            return;
        }
        if(m_selectStart == QPoint(-1,-1)){
            int l = m_ptClicked.x() / m_fontWidth;
            int t = m_ptClicked.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
            m_selectStart = m_selectEnd = QPoint(l, t + m_scrollValue);
        }
        int r = pt.x() / m_fontWidth;
        int b = pt.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
        m_selectEnd = QPoint(r, b + m_scrollValue);
        m_view->setSelection(m_selectStart, m_selectEnd);
    }
}

void QKxTermItem::mouseReleaseEvent(QMouseEvent *ev)
{
    QPoint pt = ev->pos();

    int key = 0;
    switch(ev->button()) {
    case Qt::LeftButton:
        key = 0;
        break;
    case Qt::MiddleButton:
        key = 1;
        break;
    case Qt::RightButton:
        key = 2;
        break;
    }

    int xoffset = pt.x() - m_ptClicked.x();
    int yoffset = pt.y() - m_ptClicked.y();
    if( qAbs(xoffset) < 3 && qAbs(yoffset) < 3) {
        int x = m_ptClicked.x() / m_fontWidth;
        int y = m_ptClicked.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
        x += 1;
        y += 1;

        int modifier = 0;
        if(ev->modifiers() & Qt::ShiftModifier) {
            modifier += 0x04;
        }
        if(ev->modifiers() & Qt::ControlModifier) {
            modifier += 0x10;
        }
        qDebug() << "mouseReleaseEvent" << xoffset << yoffset;
        unsigned int states = m_vte->states();
        char buf[32] = {0};
        if(states & (TF_MOUSE_9 | TF_MOUSE_1000 | TF_MOUSE_1001 | TF_MOUSE_1002 | TF_MOUSE_1003)) {
            if (x <= 223 && y <= 223) {
                int cnt = 0;
                //pressed
                cnt = sprintf(buf, "\033[M%c%c%c", modifier + key + 32, x + 32, y + 32);
                emit sendData(QByteArray(buf, cnt));
                //release
                cnt = sprintf(buf, "\033[M%c%c%c", modifier + 3 + 32, x + 32, y + 32);
                emit sendData(QByteArray(buf, cnt));
            }
        }else if(states & TF_MOUSE_1006) {
            int cnt = 0;
            cnt = sprintf(buf, "\033[<%d;%d;%d%c", modifier, x, y, 'M');
            emit sendData(QByteArray(buf, cnt));
            cnt = sprintf(buf, "\033[<%d;%d;%d%c", modifier, x, y, 'm');
            emit sendData(QByteArray(buf, cnt));
        }else if(states & TF_MOUSE_1015) {
            int cnt = 0;
            cnt = sprintf(buf, "\033[%d;%d;%dM", modifier + key + 32, x, y);
            emit sendData(QByteArray(buf, cnt));
            cnt = sprintf(buf, "\033[%d;%d;%dM", modifier + 3 + 32, x, y);
            emit sendData(QByteArray(buf, cnt));
        }
    }
}

void QKxTermItem::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ev->accept();
    if(ev->buttons() & Qt::LeftButton) {
        QPoint pt = ev->pos();
        int x = pt.x() / m_fontWidth;
        int y = pt.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
        m_view->selectWord(QPoint(x, y + m_scrollValue));
        m_tripleClick = true;
        QTimer::singleShot(500,this, SLOT(onTripleClickTimeout()));
    }
}

void QKxTermItem::wheelEvent(QWheelEvent *ev)
{
    if (ev->orientation() != Qt::Vertical){
        return;
    }    
    int lineToScroll = ev->delta() / 30;
    int vmax = m_view->historyLineCount();
    if(vmax) {
        int value = m_scrollValue;
        value -= lineToScroll;
        if(value < 0) {
            value = 0;
        }else if(value > vmax) {
            value = vmax;
        }
        updateScrollValue(value);
    }else {
        int key = ev->delta() > 0 ? Qt::Key_Up : Qt::Key_Down;
        QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
        for (int i=0; i < qAbs(lineToScroll);i++) {
            handleKeyEvent(&event);
        }
    }

    Qt::MouseButtons btns = ev->buttons();
    if((btns & Qt::LeftButton) && m_selectEnd.y() > 0 && m_selectStart.y() > 0) {
        QPoint pt = ev->pos();
         //qDebug() << "mouseMoveEvent" << rt;
         int r = pt.x() / m_fontWidth;
         int b = pt.y() / (m_fontHeight + m_spaceLine + m_lineWidth);
         m_selectEnd = QPoint(r, b + m_scrollValue);
         m_view->setSelection(m_selectStart, m_selectEnd);
    }
}

bool QKxTermItem::focusNextPrevChild(bool next)
{
    return false;
}

void QKxTermItem::inputMethodEvent(QInputMethodEvent *ev)
{
    ev->accept();
    QString tmp = ev->commitString();
    if(!tmp.isEmpty()) {
        m_preeditText.clear();
        update(m_preeditRect);
        QKeyEvent keyEvent(QEvent::KeyPress, 0, Qt::NoModifier, ev->commitString());
        handleKeyEvent(&keyEvent);
        return;
    }
    m_preeditText = ev->preeditString();
    const QPoint& pt = m_view->cursorToScreenPosition();
    QRect rt = QRect(pt.x() * m_fontWidth,
                          pt.y() * (m_fontHeight+m_spaceLine + m_lineWidth),
                          m_fontWidth * m_preeditText.length(),
                          m_fontHeight+m_spaceLine + m_lineWidth);
    QRegion rgn(rt);
    rgn |= m_preeditRect;
    m_preeditRect = rt;
    //qDebug() << "inputMethod" << m_preeditText << ev->commitString() << m_preeditRect;
    update(rgn.boundingRect());
}

QVariant QKxTermItem::inputMethodQuery(Qt::InputMethodQuery query, QVariant v) const
{
    //qDebug() << "inputMethodQuery" << query << v;
    if(m_view && inputEnable()) {
        QPoint pt = m_view->cursorToScreenPosition();
        pt = QPoint(pt.x() * m_fontWidth, (pt.y()+1) * (m_fontHeight+m_spaceLine + m_lineWidth));
        if(query == Qt::ImEnabled) {
            return 1;
        }else if(query == Qt::ImMicroFocus) {
            return QRect(pt.x(), pt.y(), 1, 1);
        }else if(query == Qt::ImFont) {
            return m_font;
        }else if(query == Qt::ImHints) {
            Qt::InputMethodHints hints;
            hints |= Qt::ImhNoAutoUppercase;
            hints |= Qt::ImhPreferLatin;
            hints |= Qt::ImhNoPredictiveText;
            return int(hints);
        }
    }

    return QWidget::inputMethodQuery(query);
}

void QKxTermItem::showInputMethod(bool show)
{
    QInputMethod *im = QGuiApplication::inputMethod();
    if(im){
        im->setVisible(show);
    }
}

bool QKxTermItem::event(QEvent *e)
{
    return QWidget::event(e);
}

bool QKxTermItem::eventFilter(QObject *obj, QEvent *ev)
{
    QEvent::Type type = ev->type();
    if(type == QEvent::Enter) {
        QLabel *label = qobject_cast<QLabel*>(obj);
        if(label) {
            resetTitlePosition(true);
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void QKxTermItem::updateView(QKxTermItem::PaintFlag flags)
{
    m_flagPaints |= flags;
    m_ticker->setSingleShot(true);
    m_ticker->start(REPAINT_TIMEOUT1);
    if(!m_ticker2->isActive()) {
        m_ticker2->setSingleShot(true);
        m_ticker2->start(REPAINT_TIMEOUT2);
    }
}

void QKxTermItem::updateScrollValue(int val)
{
    if(val > m_view->historyLineCount()) {
        val = m_view->historyLineCount();
    }
    if(val != m_scrollValue) {
        m_scrollValue = val;
        m_autoScrollToEnd = qAbs(m_view->historyLineCount() - val) < 3;
        updateView(PF_Scroll);
#ifdef MOBILE
        int total = m_view->lineCount();
        qreal sz = qreal(m_rows) / qreal(total);
        qreal pos = qreal(m_scrollValue) / qreal(total);
        emit scrollValueChanged(sz, pos);
#else
        emit scrollValueChanged(m_view->historyLineCount(), m_scrollValue);
#endif
    }
}

void QKxTermItem::updateFontInfo()
{
    QFont ft = m_font;
    ft.setUnderline(true);
    ft.setOverline(true);
    QFontMetricsF fm(ft);
    QSizeF sz = fm.size(Qt::TextSingleLine|Qt::TextDontClip, LTR_OVERRIDE_CHAR+QLatin1String(REPCHAR));
    uint cnt = qstrlen(REPCHAR);
    int w = qRound(sz.width() / cnt);
    int h = qRound(sz.height());
    int n = fm.lineWidth();
    if(n < 2) {
        n = 2;
    }
    qreal i = fm.underlinePos();
    qreal ascent = fm.ascent();
    qreal descent = fm.descent();
    m_fontWidth = w;
    m_fontHeight = h;
    m_lineWidth = n; // the width of the underline and strikeout lines, adjusted for the point size of the font.
}

void QKxTermItem::updateTermSize()
{
    m_rows = int(height()) / int(m_fontHeight + m_spaceLine + m_lineWidth);
    m_columns = int(width()) / int(m_fontWidth);
    updateTermSize(m_rows, m_columns);
    emit termSizeChanged(m_rows, m_columns);
    updateView(PF_FullScreen);
}

void QKxTermItem::updateImage()
{
    bool reverse;
    QList<TermLine> image = m_view->viewport(m_scrollValue, m_rows, m_columns, &reverse, m_selectMode);
    if(m_screenReverse != reverse) {
        m_screenReverse = reverse;
        update(QRect(0, 0, int(width()), int(height())));
        setBackgroundColor(reverse ? m_colorSchema->foreground() : m_colorSchema->background());
    }
    if(m_flagPaints.testFlag(PF_FullScreen) || m_flagPaints.testFlag(PF_Scroll)) {
        update(QRect(0, 0, int(width()), int(height())));
    }else {
        int row = qMax(image.length(), m_image.length());
        QRegion clip;
        for(int r = 0; r < row; r++) {
            if(image.length() <= r) {
                image.append(TermLine());
            }
            if(m_image.length() <= r) {
                m_image.append(TermLine());
            }
            TermLine& src = image[r];
            TermLine& dst = m_image[r];
            int col = qMax(src.cs.length(), dst.cs.length());
            if(src.cs.length() < col) {
                src.cs.resize(col);
            }
            if(dst.cs.length() < col) {
                dst.cs.resize(col);
            }
            if(src.xcur >= 0) {
                QRect rt(src.xcur * m_fontWidth, r * (m_fontHeight + m_spaceLine + m_lineWidth), m_fontWidth, m_fontHeight + m_spaceLine + m_lineWidth);
                clip |= rt.adjusted(-2, -2, 2, 2);
            }
            if(dst.xcur >= 0) {
                QRect rt(dst.xcur * m_fontWidth, r * (m_fontHeight + m_spaceLine + m_lineWidth), m_fontWidth, m_fontHeight + m_spaceLine + m_lineWidth);
                clip |= rt.adjusted(-2, -2, 2, 2);
            }
            int y = r * (m_fontHeight + m_spaceLine + m_lineWidth);
            int c = 0;
            int cnt = 0;
            while(c < col) {
                TermChar& s = src.cs[c];
                TermChar& d = dst.cs[c];
                if(s.equal(d)) {
                    cnt += s.count;
                    c++;
                    continue;
                }
                int x = cnt * m_fontWidth;
                cnt += s.count;
                c++;
                while(c < col) {
                    TermChar& s = src.cs[c];
                    TermChar& d = dst.cs[c];
                    if(s.equal(d)) {
                        break;
                    }
                    cnt += s.count;
                    c++;
                }
                int x2 = cnt * m_fontWidth;
                QRect rt(x, y, x2 - x,  m_fontHeight + m_spaceLine + m_lineWidth);
                clip |= rt;
            }
        }
        if(clip.rectCount() == 0) {
            return;
        }
        //qDebug() << "updateClip" << rts;
        update(clip);
    }
    m_cursorReverse = true;
    m_image = image;
}

void QKxTermItem::drawLine(QPainter *p, int row,  const TermLine &line)
{
    if(line.cs.isEmpty() || line.t == TS_BottomHalf) {
        return;
    }
    int col = 0;
    int y = row * (m_fontHeight + m_spaceLine + m_lineWidth);
    QTransform textScale;
    if (line.t == TS_TopHalf) {
        textScale.scale(2,2);
    }else if (line.t == TS_DoubleWidth) {
        textScale.scale(2,1);
    }
    p->setWorldTransform(textScale, true);

    if(line.xcur >= 0) {
        int i = row + 1 - 1;
    }

    QVector<TermChar>::const_iterator iter = line.cs.begin();
    while(iter != line.cs.end()){
        QString out;
        int x = col * m_fontWidth;
        TermAttributes attr = iter->attr;
        bool graphic = iter->graphic;
        int cnt = iter->count;
        if(cnt <= 0) {
            iter++;
            continue;
        }
        out.append(QChar(iter->c));
        bool bCursor = false;
        if(col == line.xcur) {
            iter++;
            bCursor = true;
        }else{
            for(iter++; iter != line.cs.end(); iter++){
                if(!attr.equal(iter->attr) || iter->count == 0 || graphic != iter->graphic || col + cnt == line.xcur) {
                    break;
                }
                out.append(QChar(iter->c));
                cnt += iter->count;
            }
        }
        col += cnt;
        //wchar_t c = out.at(out.length() - 1).unicode();
        if(!m_vte->appMode()){
            if(!(attr.flags & TermAttributes::AF_Reverse) && !bCursor && attr.bg.isDefault() && attr.fg.isDefault()) {
                for(int i = 0; i < out.length(); i++) {
                    if(out.at(i) != QChar(32)) {
                        out.remove(0, i);
                        cnt -= i;
                        x += m_fontWidth * i;
                        break;
                    }
                }
                for(int i = out.length() - 1; i > 0; i--) {
                    if(out.at(i) != QChar(32)) {
                        int n = out.length();
                        out.resize(i+1);
                        cnt -= n - i - 1;
                        break;
                    }
                }
                if(cnt == 0) {
                    continue;
                }
            }
        }

        QColor bg = m_colorSchema->bg(attr.bg);
        QColor fg = bCursor ? m_colorSchema->cursor() : m_colorSchema->fg(attr.fg);
        if(m_screenReverse) {
            qSwap(bg, fg);
            qSwap(attr.bg, attr.fg);
        }

        bool reverse = attr.flags & TermAttributes::AF_Reverse;
        bool colorful = attr.bg.isColorful();

        QRect rt(x, y, m_fontWidth * cnt, m_fontHeight);
        QRect brt(x, y, m_fontWidth * cnt, m_fontHeight + m_lineWidth);
        rt.moveTopLeft(textScale.inverted().map(rt.topLeft()));

        if (reverse) {
            qSwap(bg, fg);
            colorful = attr.fg.isColorful();
        }
        if(m_highContrast && colorful) {
            fg = m_colorSchema->pretty(bg);
        }
        if(bCursor) {
            if(m_cursorType != CT_Block) {
                p->fillRect(brt, bg);
            }
        }else{
            if(bg != m_backgroundColor) {
                p->fillRect(brt, bg);
            }
        }
        if(bCursor) {
            drawCursor(p, brt, bg, fg, reverse);
        }
        QFont ft = m_font;
        if(attr.flags & TermAttributes::AF_Invisible) {
            continue;
        }
        if(attr.flags & TermAttributes::AF_Blink) {
            m_blinkRects.append(rt.adjusted(-2, -2, 2, 2));
            if(!m_blinkShow) {
                continue;
            }
        }
        if(attr.flags & TermAttributes::AF_Bold) {
            ft.setBold(true);
        }
        if(attr.flags & TermAttributes::AF_Underline){
            ft.setUnderline(true);
        }
        // don't support italic for italic width not equal to regular width.
        if(attr.flags & TermAttributes::AF_Italic) {
            ft.setItalic(true);
        }
        if(attr.flags & TermAttributes::AF_Overline) {
            ft.setOverline(true);
        }
        p->setFont(ft);
        if(bCursor) {
            p->setPen(reverse ? bg : fg);
        }else{
            p->setPen(fg);
        }

        //qDebug() << "drawLine" << graphic << out;
        //QRect ort;
        p->drawText(rt, Qt::AlignBaseline|Qt::TextDontClip, LTR_OVERRIDE_CHAR + out);
        //p->drawText(rt, Qt::AlignBottom|Qt::TextDontClip, LTR_OVERRIDE_CHAR + out);
        //p->drawText(rt.x(), rt.bottom(), LTR_OVERRIDE_CHAR + out);
        //p->drawRect(rt);
        //p->drawRect(ort);
    }
    p->setWorldTransform(textScale.inverted(), true);
}

void QKxTermItem::drawCursor(QPainter *p, const QRect &rt, const QColor &bg, const QColor &fg, bool &inverse)
{
    //qDebug() << "drawCursor" << m_focus;
    m_cursorRect = rt.adjusted(-2, -2, 2, 2);
    if(m_cursorRectLast != m_cursorRect) {
        inverse = true;
        m_cursorLeftReverse = BLINK_MAX_COUNT;
    }else{
        inverse = m_cursorReverse;
    }
    if(m_cursorLeftReverse > (BLINK_MAX_COUNT - 3)) {
        inverse = true;
        m_cursorLeftReverse--;
    }
    m_cursorRectLast = m_cursorRect;
    if(m_cursorType == CT_Block) {
        if(m_focus || m_blinkAlway) {
            p->fillRect(rt, inverse ? fg : bg);
        }else{
            p->setPen(fg);
            p->drawRect(rt);
        }
    }else if(m_cursorType == CT_Underline) {
        if(inverse){
            QPen pen = p->pen();
            pen.setWidth(2);
            pen.setColor(fg);
            p->setPen(pen);
            p->drawLine(rt.bottomLeft(), rt.bottomRight());
        }
    }else if(m_cursorType == CT_IBeam) {
        if(inverse){
            QPen pen = p->pen();
            pen.setWidth(2);
            pen.setColor(fg);
            p->setPen(pen);
            p->drawLine(rt.topLeft(), rt.bottomLeft());
        }
    }
}

void QKxTermItem::handleKeyEvent(QKeyEvent *ev)
{
    unsigned int states = m_vte->states();
    QKxKeyTranslator::VTModes vtmode;
    vtmode |= states & TF_DECANM ? QKxKeyTranslator::VM_ANSI_NO : QKxKeyTranslator::VM_ANSI_YES;
    vtmode |= states & TF_LNM ? QKxKeyTranslator::VM_NEWLINE_YES : QKxKeyTranslator::VM_NEWLINE_NO;
    vtmode |= states & TF_DECCKM ? QKxKeyTranslator::VM_APPCUKEY_YES : QKxKeyTranslator::VM_APPCUKEY_NO;
    vtmode |= states & TF_SCREEN ? QKxKeyTranslator::VM_APPSCREEN_YES : QKxKeyTranslator::VM_APPSCREEN_NO;
    vtmode |= states & TF_DECKPAM ? QKxKeyTranslator::VM_APPKEYPAD_YES : QKxKeyTranslator::VM_APPKEYPAD_NO;
    int key = ev->key();
    Qt::KeyboardModifiers modifiers = ev->modifiers();
    QByteArray buf = m_keyTranslator->match(key, modifiers, vtmode);
    if(!buf.isEmpty()) {
        char tmp[100];
        sprintf(tmp, "0x%x", ev->key());
        qDebug() << "keyPressEvent:" << tmp  << "buf:" << buf << "text:" << ev->text();
        emit sendData(buf);
    }else{
        if(ev->key() >= 0x40 && ev->key() < 0x5f && (ev->modifiers() & Qt::ControlModifier)) {
            buf.append(ev->key() & 0x1f);
        }else if(ev->key() == Qt::Key_Tab) {
            buf.append(0x09);
        }else if(ev->key() == Qt::Key_PageUp) {
            buf.append("\033[5~");
        }else if(ev->key() == Qt::Key_PageDown) {
            buf.append("\033[6~");
        }else {
            buf.append(ev->text().toUtf8());
        }
        if(!buf.isEmpty()) {
            char tmp[100];
            sprintf(tmp, "0x%x", ev->key());
            qDebug() << "keyPressEvent" << tmp << ev->text();
            emit sendData(buf);
        }
    }
}

void QKxTermItem::scroll(int yoffset)
{
    int lineToScroll = yoffset / 5;
    int vmax = m_view->historyLineCount();
    if(vmax) {
        int value = m_scrollValue;
        value += lineToScroll;
        if(value < 0) {
            value = 0;
        }else if(value > vmax) {
            value = vmax;
        }
        updateScrollValue(value);
    }else {
        int key = yoffset > 0 ? Qt::Key_Up : Qt::Key_Down;
        QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
        for (int i=0; i < qAbs(lineToScroll);i++) {
            handleKeyEvent(&event);
        }
    }
}

bool QKxTermItem::isLineVisible(int y)
{
    if(y < m_scrollValue || y >= m_scrollValue + m_rows) {
        return false;
    }
    return true;
}
