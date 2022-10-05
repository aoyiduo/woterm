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

#include "qvteimpl.h"

#include "wcwidth.h"
#include "qkxscreen.h"
#include "qkxhistory.h"
#include "qkxutils.h"

#include <QTextCodec>
#include <QDebug>
#include <QFont>
#include <QFontMetricsF>

static const int graphic_size = 32;
//static quint16 vt100_graphics_std[graphic_size] =
//{ // 0/8     1/9    2/10    3/11    4/12    5/13    6/14    7/15
//  0x0020, 0x25C6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0,
//  0x00b1, 0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c,
//  0xF800, 0xF801, 0x2500, 0xF803, 0xF804, 0x251c, 0x2524, 0x2534,
//  0x252c, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00b7
//};

/*
 *  不要使用 vt100_graphics_std字符集，因为其字体长度不一致且大小也不一致，导致vttest charset 检测异常。
 *
*/

static wchar_t unitab_xterm_std[graphic_size] =
{
    0x0020, 0x2666, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0,
    0x00b1, 0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c,
    0x23ba, 0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534,
    0x252c, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00a3, 0x00b7
};

//https://en.wikipedia.org/wiki/Box-drawing_character
static wchar_t unitab_xterm_box[] =
{
    0x2500, //horizconnector ─ BOX DRAWINGS LIGHT HORIZONTAL */
    0x2502, //vertbar │ BOX DRAWINGS LIGHT VERTICAL */
    0x250c, //topleftradical ┌ BOX DRAWINGS LIGHT DOWN AND RIGHT */
    0x2510, //uprightcorner ┐ BOX DRAWINGS LIGHT DOWN AND LEFT */
    0x2514, //lowleftcorner └ BOX DRAWINGS LIGHT UP AND RIGHT */
    0x2518, //lowrightcorner ┘ BOX DRAWINGS LIGHT UP AND LEFT */
    0x251c, //leftt ├ BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
    0x2524, //rightt ┤ BOX DRAWINGS LIGHT VERTICAL AND LEFT */
    0x252c, //topt ┬ BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
    0x2534, //bott ┴ BOX DRAWINGS LIGHT UP AND HORIZONTAL */
    0x253c  //crossinglines ┼ BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
};

static wchar_t *vt100_graphic = unitab_xterm_std;

static wchar_t toGraphicChar(wchar_t c, const TermCursor &tc)
{
    TermCharset cs = tc.currentCharset();
    if(cs == CS_G0 && 0x5f <= c && c <= 0x7e) {
        return vt100_graphic[c-0x5f];
    }
    return c;
}

QVteImpl::QVteImpl(QObject *parent)
    : QObject(parent)
{
    m_cmdScreen = new QKxScreen(this);
    m_cmdScreen->setHistory(new QKxHistoryBuffer(this));
    m_appScreen = new QKxScreen(this);
    m_appScreen->setHistory(new QKxHistoryNone(this));
    m_screen = m_cmdScreen;
    m_codec = nullptr;
    m_code = DEFAULT_TEXT_CODEC;
}

QVteImpl::~QVteImpl()
{
    delete m_cmdScreen;
    delete m_appScreen;
}

void QVteImpl::setFont(const QFont &ft)
{
    m_font = ft;
    m_box.clear();
    m_graphicCount.clear();
}

QString QVteImpl::textCodec() const
{
    return m_code;
}

void QVteImpl::setTextCodec(const QString &code)
{
    if(m_code != code) {
        m_code = code;
        m_codec = nullptr;
    }
}


void QVteImpl::process(const QByteArray &data)
{
    if(m_codec == nullptr) {
        m_codec = QTextCodec::codecForName(m_code.toUtf8());
    }
    QByteArray buf = m_bufLeft.isEmpty() ? data : m_bufLeft + data;
    m_bufLeft.clear();
    QTextCodec::ConverterState left;
    QString utf16Text = m_codec->toUnicode(buf.data(), buf.length(), &left);
    int cnt = left.remainingChars;
    if(cnt > 0) {
        m_bufLeft = data.right(cnt);
    }
    std::wstring unicodeText = utf16Text.toStdWString();
    QVte::process(unicodeText.c_str(), int(unicodeText.length()));
    if(m_flags) {
        emit contentChanged(m_flags.testFlag(UF_FullScreen));
    }
}

QKxScreen *QVteImpl::screen()
{
    return m_screen;
}

bool QVteImpl::appMode()
{
    unsigned int _states = states();
    return _states & (TF_DECCKM | TF_SCREEN);
}

void QVteImpl::clearAll()
{
    reset();
    m_screen = m_cmdScreen;
    m_cmdScreen->clearAll();
    m_appScreen->clearAll();
}

int QVteImpl::historySize()
{
    return qMax(m_cmdScreen->history()->maxLine(),
                m_appScreen->history()->maxLine());
}

void QVteImpl::setHistorySize(int s)
{
    m_cmdScreen->history()->setMaxLine(s);
    m_appScreen->history()->setMaxLine(s);
}

void QVteImpl::setHistoryFile(const QString &file)
{
    m_cmdScreen->setHistoryFile(file);
}

void QVteImpl::stopHistoryFile()
{
    m_cmdScreen->stopHistoryFile();
}

void QVteImpl::cleanHistory()
{
    m_cmdScreen->cleanHistory();
}

void QVteImpl::resetState()
{
    m_screen = m_cmdScreen;
    m_appScreen->clearScreen();
    m_cmdScreen->clearScreen();
    reset();
    m_bufLeft.clear();
}

void QVteImpl::screenResize(int rows, int cols)
{
    m_appScreen->resize(rows, cols);
    m_cmdScreen->resize(rows, cols);
    m_flags |= UF_FullScreen;
}

void QVteImpl::resetColor(int c)
{
    m_screen->resetColor(c);
}

void QVteImpl::defineColor(int c, const wchar_t *s)
{
    m_screen->defineColor(c, s);
}

void QVteImpl::defineCharset(int n, TermCharset cs)
{
    m_screen->defineCharset(n, cs);
}

void QVteImpl::selectCharset(int n)
{
    m_screen->selectCharset(n);
}

void QVteImpl::showCursor(const TermCursor &tc)
{
    m_screen->showCursor(tc);
}

void QVteImpl::hideCursor(const TermCursor &tc)
{
    m_screen->hideCursor(tc);
}

void QVteImpl::clearTabs()
{
    m_cmdScreen->clearTabs();
    m_appScreen->clearTabs();
}

void QVteImpl::setTab(int x)
{
    m_cmdScreen->setTab(x);
    m_appScreen->setTab(x);
}

void QVteImpl::resetTab(int x)
{
    m_cmdScreen->resetTab(x);
    m_appScreen->resetTab(x);
}

int QVteImpl::getNextTab(int x, int n)
{
    return m_screen->getNextTab(x, n);
}

int QVteImpl::getCharWidth(wchar_t c, const TermCursor &tc)
{
    int cnt = 0;
    if(tc.graphicCharset()) {
        c = toGraphicChar(c, tc);
    }
    cnt = specialCharactorCount(c);
    return cnt > 0 ? cnt : countOfChar(c);
}

void QVteImpl::putReversedQuestionMark(const TermCursor &tc)
{
    m_screen->putReversedQuestionMark(tc);
}

void QVteImpl::putChar(wchar_t c, int char_width, const TermCursor &tc)
{
    //qDebug() << "putChar" << QChar(c) << char_width << cursor.x << cursor.y;
    if(tc.graphicCharset()) {
        c = toGraphicChar(c, tc);
        //qDebug() << "putChar" << QChar(c) << char_width << tc.x << tc.y;
    }
    //if(c == 65533) {
        //qDebug() << "putChar" << QChar(c) << char_width << tc.x << tc.y;
    //}
    m_screen->putChar(c, char_width, tc);
    m_flags |= UF_Normal;
}

bool QVteImpl::requestWidth(int width)
{
    return false;
}

void QVteImpl::clearRegion(int x1, int y1, int x2, int y2, const TermCursor &tc)
{
    m_screen->clearRegion(x1, y1, x2, y2, tc);
    m_flags |= UF_Normal;
}

void QVteImpl::insertChars(int x, int n, const TermCursor &tc)
{
    m_screen->insertChars(x, n, tc);
    m_flags |= UF_Normal;
}

void QVteImpl::deleteChars(int x, int n, const TermCursor &tc)
{
    m_screen->deleteChars(x, n, tc);
    m_flags |= UF_Normal;
}

void QVteImpl::scrollUp(int y1, int y2, int n)
{
    m_screen->scrollUp(y1, y2, n);
    m_flags |= UF_FullScreen;
}

void QVteImpl::scrollDown(int y1, int y2, int n)
{
    m_screen->scrollDown(y1, y2, n);    
    m_flags |= UF_FullScreen;
}

void QVteImpl::bell()
{
    qDebug() << "bell";
}

void QVteImpl::setLineTextSize(int y, TermTextSize t)
{
    m_screen->setLineTextSize(y, t);
}

void QVteImpl::ttyWrite(const char *s)
{
    emit sendData(s);
}

void QVteImpl::setWindowTitle(const wchar_t *title)
{
    QString tmp = QString::fromWCharArray(title);
    emit titleChanged(tmp);
}

void QVteImpl::flagLockKeyboard(bool state)
{
    qDebug() << "flagLockKeyboard" << state;
}

void QVteImpl::flagApplicationScreen(bool state)
{
    if(state) {
        m_appScreen->clearAll();
        m_screen = m_appScreen;
    }else {
        m_screen = m_cmdScreen;
    }
    m_flags |= UF_FullScreen;
    emit screenChanged();
   // qDebug() << "flag_application_keypad(state:" << state << ")";
}

void QVteImpl::flagApplicationKeypad(bool state)
{
    //qDebug() << "flag_application_keypad(state:" << state << ")";
}

void QVteImpl::flagCursorVisible(bool state)
{
    m_screen->flagCursorVisible(state);
}

void QVteImpl::flagSoftScroll(bool state)
{
    qDebug() << "flag_cursor_visible(state:" << state << ")";
}

void QVteImpl::flagScreenReversed(bool state)
{
    qDebug() << "flag_screen_reversed(state:" << state << ")";
    m_screen->flagScreenReversed(state);
    m_flags |= UF_FullScreen;
}

void QVteImpl::saveCursor(const TermCursor &cursor)
{
    m_screen->saveCursor(cursor);
}

TermCursor QVteImpl::restoreCursor() const
{
    return m_screen->restoreCursor();
}

void QVteImpl::updateCursor(const TermCursor &cursor)
{
    m_screen->updateCursor(cursor);
}

TermCursor QVteImpl::cursorNow() const
{
    return m_screen->cursorNow();
}

int QVteImpl::graphicCount(wchar_t c)
{
    if(m_graphicCount.isEmpty()) {
        QFontMetricsF fm(m_font);
        int fw = fm.width("W");
        for(int i = 0; i < graphic_size; i++) {
            int ch = vt100_graphic[i];
            qreal w = fm.width(ch);
            int cnt = qRound(w / fw);
            m_graphicCount.insert(ch, cnt);
            //qDebug() << ch << cnt;
        }
    }
    return m_graphicCount.value(c, -1);
}

int QVteImpl::specialCharactorCount(wchar_t c)
{
    if(m_box.isEmpty()) {
        QFontMetrics fm(m_font);

        int fw = fm.width("W");
        for(int i = 0; i < graphic_size; i++) {
            int ch = vt100_graphic[i];
            int w = fm.width(ch);
            m_box.insert(ch, qRound(float(w) / fw));
            //qDebug() << ch << cnt;
        }
        //https://en.wikipedia.org/wiki/Box-drawing_character        
        // box draw
        for(int i = 0x2500; i <= 0x257F; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
            //qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
        }

        //block draw
        for(int i = 0x2580; i <= 0x259F; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
            //qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
        }

        // special symbols
        for(int i = 0x1FB00; i <= 0x1FBFF; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
        }
    }
    return m_box.value(c, 0);
}
