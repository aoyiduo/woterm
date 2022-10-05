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

#include "qvte.h"

#include <QTextCodec>
#include <QDebug>

static inline bool isControlC0(wchar_t c) {
    return (c <= 0x1f);
}

static inline bool isControlC1(wchar_t c) {
    return (c >= 0x80 && c <= 0x9f);
}

static inline bool isControl(wchar_t c) {
    return isControlC0(c) || isControlC1(c);
}

static inline bool isDel(wchar_t c) {
    return c == 0x7f;
}

static inline bool isParameterSeparator(wchar_t c) {
    return (c == ';');
}

static inline bool isParameter(wchar_t c) {
    return ((c >= '0' && c <= '9'));
}

static inline bool hasFlag(unsigned int v, unsigned int flag) {
    return (v & flag) > 0;
}

template<typename T> static inline void setRange(T& v, T l, T h) {
    if (v < l) {
        v = l;
    }
    if (v > h) {
        v = h;
    }
}

template<typename T> static inline T getDefault(T v, T d) {
    return v < 1 ? d : v;
}

static int wtoi(wchar_t *s) {
    int t = 0;
    while(*s != 0) {
        wchar_t c = *s;
        t = 10 * t + (c - '0');
        s++;
    }
    return t;
}

// * mode modifiers ************************************************************

static inline void setFlag(unsigned int& mode, unsigned int flag) {
    mode |= flag;
}

static inline void resetFlag(unsigned int& mode, unsigned int flag) {
    mode &= ~flag;
}

typedef void (*ModeModifier)(unsigned int& mode, unsigned int flag);

QVte::QVte()
{
    columns = 80;
    rows = 40;
    top = 0;
    bottom = rows - 1;
    mode = TM_Ground;

    // according to the datasheet of DEC, DECAWM is default OFF.
    // but the linux console defaults to ON.
    // http://vt100.net/docs/vt510-rm/chapter2.html#S2.13
    flags = TF_DECAWM | TF_DECTCEM | TF_DECARM | TF_DECSCLM | TF_UTF8;
    cursor_hidden = true;    
}

QVte::~QVte(){

}

void QVte::process(const wchar_t *buf, int length)
{
    for(int i = 0; i < length; i++) {
        process(buf[i]);
    }
}

void QVte::resize(int rows, int cols)
{
    if(rows <= 5) {
        rows = 5;
    }
    if(cols < 60) {
        cols = 60;
    }
    this->columns = cols;
    this->rows = rows;

    top = 0;
    bottom = rows - 1;
    screenResize(rows, cols);
    clearTabs();
    for (int x = 8; x < cols; x += 8) {
        setTab(x);
    }
    cursor = cursorNow();
    cursor.attrs.reset();
}

void QVte::reset()
{
    resize(rows, columns);
    mode = TM_Ground;
    flags = TF_DECAWM | TF_DECTCEM | TF_DECARM | TF_DECSCLM | TF_UTF8;
    cursor_hidden = true;
    cursor = TermCursor();
    csi_data = ControlData();
    str_data = StringData();
    defining_charset = 0;
}

unsigned int QVte::states()
{
    return flags;
}

void QVte::RIS()
{
    top = 0;
    bottom = rows - 1;
    clearTabs();
    for (int x = 8; x < columns; x += 8) {
        setTab(x);
    }
    cursor.reset();
    mode = TM_Ground;

    // according to the datasheet of DEC, DECAWM is default OFF.
    // but the linux console defaults to ON.
    // http://vt100.net/docs/vt510-rm/chapter2.html#S2.13
    flags = TF_DECAWM | TF_DECTCEM | TF_DECARM | TF_DECSCLM | TF_UTF8;

    saveCursor(cursor);
    resetEscape();
    clearRegion(0, 0, columns - 1, rows - 1, cursor);
    cursor_hidden = true;
    notifyKAM();
    notifyKPAM();
    notifyDECSCNM();
    notifyDECSCLM();
    str_data.reset();

    // reset all colors
    for (int i = 0; i < 256; ++i) {
        resetColor(i);
    }

    // setup charsets
    defining_charset = 0;
    for (int i = 0; i < CharsetTableSize; ++i) {
        defineCharset(i, cursor.charset_table[i]);
    }
    selectCharset(cursor.selected_charset);

    // show cursor
    flagCursorVisible(hasFlag(flags, TF_DECTCEM));
    sc();
}

void QVte::LFVT()
{
    int y = cursor.y;
    if (cursor.y == bottom) {
        trmScrollUp(top, 1);
    } else {
        y++;
    }
    moveCursor(cursor.x, y);
}

bool QVte::NEL()
{
    int y = cursor.y;
    if (cursor.y == bottom) {
        trmScrollUp(top, 1);
    } else {
        y++;
    }
    bool lnm = hasFlag(flags, TF_LNM);
    moveCursor(lnm ? cursor.x : 0, y);
    return lnm;
}

void QVte::IND()
{
    if (cursor.y == bottom) {
        trmScrollUp(top, 1);
    } else {
        moveCursor(cursor.x, cursor.y + 1);
    }
}

void QVte::RI()
{
    if (cursor.y == top) {
        trmScrollDown(top, 1);
    } else {
        moveCursor(cursor.x, cursor.y - 1);
    }
}

void QVte::DECID()
{
    ttyWrite("\033[?6c");
}

void QVte::DECSC()
{
    saveCursor(cursor);
}

void QVte::DECRC()
{
    hc();
    cursor = restoreCursor();
    setRange(cursor.y, top, bottom);
    for (int i = 0; i < CharsetTableSize; ++i) {
        defineCharset(i, cursor.charset_table[i]);
    }
    selectCharset(cursor.selected_charset);
    sc();
}

void QVte::CPR()
{
    char cpr_buffer[32];
    qsnprintf(cpr_buffer, sizeof(cpr_buffer), "\033[%i;%iR", cursor.y + 1, cursor.x + 1);
    ttyWrite(cpr_buffer);
}

void QVte::DECALN() {
    hc();
    TermCursor cursor_save = cursor;
    cursor.reset();
    for (cursor.x = 0; cursor.x < columns; ++cursor.x) {
        for (cursor.y = 0; cursor.y < rows; ++cursor.y) {
            putChar('E', 1, cursor);
        }
    }
    cursor = cursor_save;
    sc();
}

bool QVte::handleString(wchar_t c)
{
    // abort (BEL, CAN, SUB, ESC or C1)
    if (c == 0x07 || c == 0x18 || c == 0x1a || c == 0x1b || isControlC1(c)) {
        resetEscape();
        str_data.setFinished();
        return false;
    }
    // abort if buffer full
    if (!str_data.addChar(c)) {
        resetEscape();
        str_data.reset();
    }
    return true;
}

void QVte::handleControl(wchar_t c)
{
    switch (c) {
    case 0x07:      // BEL
        if (str_data.isFinished()) {
            parseString();
        } else {
            bell();
        }
        resetEscape();
        break;
    case 0x08:      // BS
        moveCursor(cursor.x - 1, cursor.y);
        break;
    case 0x09:      // HT
        moveCursor(getNextTab(cursor.x, 1), cursor.y);
        break;
    case 0x0a:      // LF
    case 0x0b:      // VT        
        LFVT();
        break;
    case 0x0d:      // CR
        if (cursor.x) {
            moveCursor(0, cursor.y);
        }
        break;
    case 0x0e:      // SO
        cursor.selected_charset = 1;
        selectCharset(cursor.selected_charset);
        break;
    case 0x0f:      // SI
        cursor.selected_charset = 0;
        selectCharset(cursor.selected_charset);
        break;
    case 0x1a:      // SUB
        setCharPre(1);
        hc();
        putReversedQuestionMark(cursor);
        sc();
        setCharPost(1);
        resetEscape();
        str_data.reset();
        break;
    case 0x18:      // CAN
        resetEscape();
        str_data.reset();
        break;
    case 0x1b:      // ESC introducer
        mode = TM_ESC;
        break;
    case 0x7f:      // DEL (ignored)
        break;
    case 0x84:      // IND (obsolete)
        IND();
        break;
    case 0x85:      // NEL
        NEL();
        resetEscape();
        break;
    case 0x88:      // HTS
        setTab(cursor.x);
        break;
    case 0x8d:      // RI
        RI();
        break;
    case 0x90:      // DCS introducer
        resetEscape();
        str_data.reset(StringData::ST_DCS);
        mode = TM_String;
        break;
    case 0x9a:      // DECID
        DECID();
        resetEscape();
        break;
    case 0x9b:      // CSI introducer
        resetEscape();
        mode = TM_CSI;
        break;
    case 0x9d:      // OSC introducer
        resetEscape();
        str_data.reset(StringData::ST_OSC);
        mode = TM_String;
        break;
    case 0x9e:      // PM introducer
        resetEscape();
        str_data.reset(StringData::ST_PM);
        mode = TM_String;
        break;
    case 0x9f:      // APC introducer
        resetEscape();
        str_data.reset(StringData::ST_APC);
        mode = TM_String;
        break;
    }
}

void QVte::handleEsc(wchar_t c)
{
    switch (c) {
    case '[':       // CSI introducer
        resetEscape();
        mode = TM_CSI;
        return;
    case '#':       // HASH introducer
        resetEscape();
        mode = TM_Hash;
        return;
    case '%':       // UTF8
        resetEscape();
        mode = TM_UTF8;
        return;
    case 'P':       // DCS introducer
        resetEscape();
        str_data.reset(StringData::ST_DCS);
        mode = TM_String;
        return;
    case '_':       // APC introducer
        resetEscape();
        str_data.reset(StringData::ST_APC);
        mode = TM_String;
        return;
    case '^':       // PM introducer
        resetEscape();
        str_data.reset(StringData::ST_PM);
        mode = TM_String;
        return;
    case ']':       // OSC introducer
        resetEscape();
        str_data.reset(StringData::ST_OSC);
        mode = TM_String;
        return;
    case 'k':       // set window title
        resetEscape();
        str_data.reset(StringData::ST_Title);
        mode = TM_String;
        break;
    case 'n':       // LS2
        cursor.selected_charset = 2;
        selectCharset(cursor.selected_charset);
        break;
    case 'o':       // LS3
        cursor.selected_charset = 3;
        selectCharset(cursor.selected_charset);
        break;
    case 'D':       // IND
        IND();
        break;
    case 'E':       // NEL
        NEL();
        break;
    case 'H':       // HTS
        setTab(cursor.x);
        break;
    case 'M':       // RI
        RI();
        break;
    case 'Z':       // DECID
        DECID();
        break;
    case 'c':       // RIS
        RIS();
        break;
    case '=':       // DECPAM
        flags |= TF_DECKPAM;
        notifyKPAM();
        break;
    case '>':       // DECPNM
        flags &= ~TF_DECKPAM;
        notifyKPAM();
        break;
    case '7':       // DECSC
        DECSC();
        break;
    case '8':       // DECRC
        DECRC();
        break;
    case '(':       // G0
    case ')':       // G1
    case '*':       // G2
    case '+':       // G3
        defining_charset = c - '(';
        resetEscape();
        mode = TM_Charset;
        return;
    case '\\':      // ST
        if (str_data.isFinished()) {
            parseString();
        }
        break;
    }
    resetEscape();
}

void QVte::handleCsi(wchar_t c)
{
    if (csi_data.spaceLeft()) {
        if (!csi_data.addArg(c)) {
            switch (c) {
            case '@':   // ICH
                trmInsertChars(cursor.x, getDefault(csi_data.getArg(0), 1));
                break;
            case 'A':   // CUU
            {
                int y = cursor.y - getDefault(csi_data.getArg(0), 1);
                setRange(y, top, bottom);
                moveCursor(cursor.x, y);
                break;
            }
            case 'B':   // CUD
            case 'e':   // VPR
            {
                int y = cursor.y + getDefault(csi_data.getArg(0), 1);
                setRange(y, top, bottom);
                moveCursor(cursor.x, y);
                break;
            }
            case 'c':   // DA
                if (csi_data.getArg(0) == 0) {
                    DECID();
                }
                break;
            case 'C':   // CUF
            case 'a':   // HPR
                moveCursor(cursor.x + getDefault(csi_data.getArg(0), 1), cursor.y);
                break;
            case 'D':   // CUB
                moveCursor(cursor.x - getDefault(csi_data.getArg(0), 1), cursor.y);
                break;
            case 'E':   // CNL
                moveCursor(0, cursor.y + getDefault(csi_data.getArg(0), 1));
                break;
            case 'F':   // CPL
                moveCursor(0, cursor.y - getDefault(csi_data.getArg(0), 1));
                break;
            case 'g':   // TBC
                switch (csi_data.getArg(0)) {
                case 0:     // delete current tab
                    resetTab(cursor.x);
                    break;
                case 3:     // delete all tabs
                    clearTabs();
                    break;
                }
                break;
            case 'G':   // CHA
            case '`':   // HPA
                moveCursor(getDefault(csi_data.getArg(0), 1) - 1, cursor.y);
                break;
            case 'H':   // CUP
            case 'f':   // HVP
                moveCursorAbsolute(
                            getDefault(csi_data.getArg(1), 1) - 1,
                            getDefault(csi_data.getArg(0), 1) - 1
                            );
                break;
            case 'I':   // CHT
                moveCursor(getNextTab(cursor.x, getDefault(csi_data.getArg(0), 1)), cursor.y);
                break;
            case 'J':   // ED
                switch (csi_data.getArg(0)) {
                case 0:     // below
                    trmClearToEndOfScreen();
                    break;
                case 1:     // above
                    trmClearToBeginOfScreen();
                    break;
                case 2:     // all
                    trmClearEntireScreen();
                    break;
                case 3:
                    // clear history
                    trmClearHistory();
                    break;
                }
                break;
            case 'K':   // EL
                switch (csi_data.getArg(0)) {
                case 0:     // right
                    trmClearToEndOfLine();
                    break;
                case 1:     // left
                    trmClearToBeginOfLine();
                    break;
                case 2:     // all
                    trmClearEntireLine();
                    break;;
                }
                break;
            case 'S':   // SU
                trmScrollUp(top, getDefault(csi_data.getArg(0), 1));
                break;
            case 'T':   // SD
                trmScrollDown(top, getDefault(csi_data.getArg(0), 1));
                break;
            case 'L':   // IL
                insertLines(cursor.y, getDefault(csi_data.getArg(0), 1));
                break;
            case 'l':   // RM
                resetModes();
                break;
            case 'M':   // DL
                deleteLines(cursor.y, getDefault(csi_data.getArg(0), 1));
                break;
            case 'X':   // ECH
                trmClearRegion(
                            cursor.x,
                            cursor.y,
                            cursor.x + getDefault(csi_data.getArg(0), 1),
                            cursor.y
                            );
                break;
            case 'P':   // DCH
                trmDeleteChars(cursor.x, getDefault(csi_data.getArg(0), 1));
                break;
            case 'Z':   // CBT
                moveCursor(getNextTab(cursor.x, -getDefault(csi_data.getArg(0), 1)), cursor.y);
                break;
            case 'd':   // VPA
                moveCursorAbsolute(cursor.x, getDefault(csi_data.getArg(0), 1) - 1);
                break;
            case 'h':   // SM
                setModes();
                break;
            case 'm':   // SGR
                setAttributes();
                break;
            case 'n':   // DSR
                switch (csi_data.getArg(0)) {
                case 0:
                    break;
                case 5: // DS
                    ttyWrite("\x1b[0n");
                    break;
                case 6: // CPR
                    CPR();
                    break;
                }
                break;
            case 'r':   // DECSTBM
                if (!csi_data.decMode()) {
                    int t = getDefault(csi_data.getArg(0), 1) - 1;
                    int b = getDefault(csi_data.getArg(1), rows) - 1;
                    if (b > t) {
                        top = t;
                        bottom = b;
                        moveCursorAbsolute(0, 0);
                    }
                }else{
                    restoreModes();
                }
                break;
            case 's':   // DECSC
                if(csi_data.decMode()) {
                    saveModes();
                }
                break;
            case 'u':   // DECRC
                DECRC();
                break;
            case 0x0b:
                qDebug() << "here";
                break;
            }
            resetEscape();
        }
    } else {
        resetEscape();
    }
}

void QVte::handleHash(wchar_t c)
{
    switch (c) {
    case '3':       // , Double-height letters, top half
        setLineTextSize(cursor.y, TS_TopHalf);
        break;
    case '4':       // DECDHL, Double-height letters, bottom half
        setLineTextSize(cursor.y, TS_BottomHalf);
        break;
    case '5':       // DECSWL, Single width, single height letters
        setLineTextSize(cursor.y, TS_Normal);
        break;
    case '6':       // DECDWL, Double width, single height letters
        setLineTextSize(cursor.y, TS_DoubleWidth);
        break;
    case '8':
        DECALN();
        break;
    }
    resetEscape();
}

void QVte::handleCharset(wchar_t c)
{
    switch (c) {
    case 'A':       // British
        cursor.charset_table[defining_charset] = CS_UK;
        defineCharset(defining_charset, cursor.charset_table[defining_charset]);
        break;
    case 'B':       // default ISO 8891 (US ASCII)
        cursor.charset_table[defining_charset] = CS_US;
        defineCharset(defining_charset, cursor.charset_table[defining_charset]);
        break;
    case '0':       // VT100 graphics
        cursor.charset_table[defining_charset] = CS_G0;
        defineCharset(defining_charset, cursor.charset_table[defining_charset]);
        break;
    case 'U':       // null, straight ROM
        cursor.charset_table[defining_charset] = CS_US;
        defineCharset(defining_charset, cursor.charset_table[defining_charset]);
        break;
    case 'K':       // user
        cursor.charset_table[defining_charset] = CS_US;
        defineCharset(defining_charset, cursor.charset_table[defining_charset]);
        break;
    }
    resetEscape();
}

void QVte::handleUtf8(wchar_t c)
{
    switch (c) {
    case '@':       // Select default (ISO 646 / ISO 8859-1)
        resetFlag(flags, TF_UTF8);
        break;
    case 'G':       // Select UTF-8
        setFlag(flags, TF_UTF8);
        break;
    }
    resetEscape();
}

void QVte::handleDefault(wchar_t c)
{
    int cnt = getCharWidth(c, cursor);
    if(cnt > 0) {
        setCharPre(cnt);
        setChar(c, cnt);
        setCharPost(cnt);
    }
}

void QVte::parseString()
{
    switch (str_data.type()) {
    case StringData::ST_OSC:
        if (str_data.parse(str_data.data(), 1) == 1) {
            int f = wtoi(str_data.getArg(0));
            switch (f) {
            case 0:     // set window title
            case 1:
            case 2:
                setWindowTitle(str_data.getArg(1));
                break;
            case 4:     // define color
                if (str_data.parse(str_data.getArg(1), 1) == 1) {
                    defineColor(wtoi(str_data.getArg(1)), str_data.getArg(2));
                }
                break;
            case 104:   // reset color
                resetColor(wtoi(str_data.getArg(1)));
                break;
            default:
                break;
            }
        }
        break;
    case StringData::ST_Title:
        if (str_data.parse(str_data.data(), 1) == 1) {
            setWindowTitle(str_data.getArg(0));
        }
        break;
    case StringData::ST_None:
    case StringData::ST_APC:
    case StringData::ST_DCS:
    case StringData::ST_PM:
        // ignored
        break;
    }
    str_data.reset();
}

void QVte::resetEscape()
{
    csi_data.reset();
    mode = TM_Ground;
}

void QVte::hc()
{
    if (!cursor_hidden) {
        hideCursor(cursor);
        cursor_hidden = true;
    }
}

void QVte::sc()
{
    if (hasFlag(flags, TF_DECTCEM)) {
         if (cursor_hidden) {
             showCursor(cursor);
             cursor_hidden = false;
         }
     }
}

void QVte::setCharPre(int char_width)
{
    if (cursor.cs == TermCursor::CS_WrapNext) {
        if (hasFlag(flags, TF_DECAWM)) {
            if (NEL()) {
                moveCursor(0, cursor.y);                
            }else{
                cursor.cs = TermCursor::CS_WrapNext;
            }
        }
    }
    if (hasFlag(flags, TF_IRM)) {
        trmInsertChars(cursor.x, char_width);
    }
}

void QVte::setChar(wchar_t c, int char_width)
{
    hc();
    putChar(c, char_width, cursor);
    sc();
}

void QVte::setCharPost(int char_width)
{
    if (cursor.x >= columns - char_width) {
        cursor.cs = TermCursor::CS_WrapNext;
    } else {
        moveCursor(cursor.x + char_width, cursor.y);
    }
}

void QVte::moveCursor(int x, int y)
{
    hc();
    cursor.x = x;
    cursor.y = y;
    if (hasFlag(flags, TF_DECOM)) {
        setRange(cursor.x, 0, columns - 1);
        setRange(cursor.y, top, bottom);
    } else {
        setRange(cursor.x, 0, columns - 1);
        setRange(cursor.y, 0, rows - 1);
    }
    cursor.cs = TermCursor::CS_Normal;
    updateCursor(cursor);
    sc();
}

void QVte::moveCursorAbsolute(int x, int y)
{
    bool on = hasFlag(flags, TF_DECOM);
    moveCursor(x, (y + (on ? top : 0)));
}

void QVte::insertLines(int y, int n)
{
    trmScrollDown(y, n);
}

void QVte::deleteLines(int y, int n)
{
    trmScrollUp(y, n);
}

void QVte::trmInsertChars(int x, int n)
{
    hc();
    if (n > columns) {
        n = columns;
    }
    insertChars(x, n, cursor);
    sc();
}

void QVte::trmDeleteChars(int x, int n)
{
    hc();
    if (n > columns) {
        n = columns;
    }
    deleteChars(x, n, cursor);
    sc();
}

void QVte::trmClearRegion(int x1, int y1, int x2, int y2)
{
    hc();
    setRange(y1, 0, rows - 1);
    setRange(y2, 0, rows - 1);
    clearRegion(x1, y1, x2, y2, cursor);
    sc();
}

void QVte::trmClearEntireLine()
{
    trmClearRegion(0, cursor.y, columns - 1, cursor.y);
}

void QVte::trmClearToEndOfLine()
{
    trmClearRegion(cursor.x, cursor.y, columns - 1, cursor.y);
}

void QVte::trmClearToBeginOfLine()
{
    trmClearRegion(0, cursor.y, cursor.x, cursor.y);
}

void QVte::trmClearToEndOfScreen()
{
    trmClearRegion(cursor.x, cursor.y, columns - 1, cursor.y);
    if (cursor.y < rows - 1) {
        trmClearRegion(0, cursor.y + 1, columns - 1, rows - 1);
    }
}

void QVte::trmClearToBeginOfScreen()
{
    if (cursor.y > 0) {
        trmClearRegion(0, 0, columns - 1, cursor.y - 1);
    }
    trmClearRegion(0, cursor.y, cursor.x, cursor.y);
}

void QVte::trmClearEntireScreen()
{
    hc();
    scrollUp(top, bottom, bottom - top + 1);
    trmClearRegion(0, 0, columns, rows-1);
    sc();
}

void QVte::trmClearHistory()
{
    // not impletement
}

void QVte::trmScrollUp(int y, int n)
{
    hc();
    scrollUp(y, bottom, n);
    cursor.x = 0;
    sc();
}

void QVte::trmScrollDown(int y, int n)
{
    hc();
    scrollDown(y, bottom, n);
    cursor.x = 0;
    sc();
}

void QVte::setColor(int &n, TermColor &color)
{
    n++;
    if (n < csi_data.argc) {
        switch (csi_data.getArg(n)) {
        case 2:     // RGB
        {
            if (n + 3 < csi_data.argc) {
                int r = csi_data.getArg(n + 1);
                int g = csi_data.getArg(n + 2);
                int b = csi_data.getArg(n + 3);
                if (r >= 0 && r < 256 && g >= 0 && g < 256 && b >= 0 && b < 256) {
                    color.type = TermColor::CT_Truecolor;
                    color.color = 1 << 24 | r << 16 | g << 8 | b;
                }
                n += 3;
            }
            break;
        }
        case 5:     // indexed
            if (n + 1 < csi_data.argc) {
                color.type = TermColor::CT_Indexed;
                color.color = csi_data.getArg(n + 1);
                n += 1;
            }
            break;
        }
    }
}

void QVte::setAttributes()
{
    // TODO: incomplete
    for (int i = 0; i < csi_data.argc; ++i) {
        int v = csi_data.getArg(i);
        switch (v) {
        case 0:
            cursor.attrs.reset();
            break;
        case 1:
            cursor.attrs.flags |= TermAttributes::AF_Bold;
            break;
        case 2:
            cursor.attrs.flags |= TermAttributes::AF_HalfBright;
            break;
        case 3:
            cursor.attrs.flags |= TermAttributes::AF_Italic;
            break;
        case 4:
            cursor.attrs.flags |= TermAttributes::AF_Underline;
            break;
        case 5:
        case 6:
            cursor.attrs.flags |= TermAttributes::AF_Blink;
            break;
        case 7:
            cursor.attrs.flags |= TermAttributes::AF_Reverse;
            break;
        case 8:
            cursor.attrs.flags |= TermAttributes::AF_Invisible;
            break;
        case 53:
            cursor.attrs.flags |= TermAttributes::AF_Overline;
            break;
        case 22:
            cursor.attrs.flags &= ~(TermAttributes::AF_Bold | TermAttributes::AF_HalfBright);
            break;
        case 23:
            cursor.attrs.flags &= ~TermAttributes::AF_Italic;
            break;
        case 24:
            cursor.attrs.flags &= ~TermAttributes::AF_Underline;
            break;
        case 25:
            cursor.attrs.flags &= ~TermAttributes::AF_Blink;
            break;
        case 27:
            cursor.attrs.flags &= ~TermAttributes::AF_Reverse;
            break;
        case 28:
            cursor.attrs.flags &= ~TermAttributes::AF_Invisible;
            break;
        case 55:
            cursor.attrs.flags &= ~ TermAttributes::AF_Overline;
            break;
        case 38:
            setColor(i, cursor.attrs.fg);
            break;
        case 39:
            cursor.attrs.fg.reset();
            break;
        case 48:
            setColor(i, cursor.attrs.bg);
            break;
        case 49:
            cursor.attrs.bg.reset();
            break;
        default:
            if (v >= 30 && v <= 37) {
                cursor.attrs.fg.type = TermColor::CT_Indexed;
                cursor.attrs.fg.color = v - 30;
            } else if (v >= 40 && v <= 47) {
                cursor.attrs.bg.type = TermColor::CT_Indexed;
                cursor.attrs.bg.color = v - 40;
            } else if (v >= 90 && v <= 97) {
                cursor.attrs.fg.type = TermColor::CT_Indexed;
                cursor.attrs.fg.color = v - 90 + 8;
            } else if (v >= 100 && v <= 107) {
                cursor.attrs.bg.type = TermColor::CT_Indexed;
                cursor.attrs.bg.color = v - 100 + 8;
            }
            break;
        }
    }
}

void QVte::restoreModes()
{
    qDebug() << csi_data.argc << csi_data.argv;
}

void QVte::saveModes()
{
    qDebug() << csi_data.argc << csi_data.argv;
    DECSC();
}

void QVte::setModes()
{
    for (int i = 0; i < csi_data.argc; ++i) {
        int v = csi_data.getArg(i);
        switch (v) {
        case 1:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECCKM);
            }
            break;
        case 2:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECANM);
            }else{
                setFlag(flags, TF_KAM);
                notifyKAM();
            }
            break;
        case 3:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECCOLM);
                notifyDECCOLM();
            }
            break;
        case 4:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECSCLM);
                notifyDECSCLM();
            }else{
                setFlag(flags, TF_IRM);
            }
            break;
        case 5:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECSCNM);
                notifyDECSCNM();
            }
            break;
        case 6:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECOM);
                notifyDECOM();
            }
            break;
        case 7:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECAWM);
            }
            break;
        case 8:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECARM);
            }
            break;
        case 12:
            if(!csi_data.decMode()) {
                setFlag(flags, TF_SRM);
            }
            break;
        case 18:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECPFF);
            }
            break;
        case 19:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECPEX);
            }
            break;
        case 20:
            if(!csi_data.decMode()) {
                setFlag(flags, TF_LNM);
            }
            break;
        case 25:
            if(csi_data.decMode()) {
                setFlag(flags, TF_DECTCEM);
                notifyTCEM();
            }
            break;
        case 47:
        case 1047:
        case 1049:
            if(csi_data.decMode()) {
                setFlag(flags, TF_SCREEN);
                saveCursor(cursor);
                notifySCREEN();
            }
            break;
        case 1048:
            saveCursor(cursor);
            break;
        case 9: // SET_X10_MOUSE
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_9);
            }
            break;
        case 1000: //SET_VT200_MOUSE
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1000);
            }
            break;
        case 1001:  //SET_VT200_HIGHLIGHT_MOUSE
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1001);
            }
            break;
        case 1002:  //SET_BTN_EVENT_MOUSE
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1002);
            }
            break;
        case 1003: //SET_ANY_EVENT_MOUSE
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1003);
            }
            break;
        case 1006: //xterm extended mouse
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1006);
            }
            break;
        case 1015: //urxvt extended mouse
            if(csi_data.decMode()) {
                setFlag(flags, TF_MOUSE_1015);
            }
            break;
        default:
            //qDebug() << "setModes:v" << v;
            break;
        }
    }
}

void QVte::resetModes()
{
    for (int i = 0; i < csi_data.argc; ++i) {
        int v = csi_data.getArg(i);
        switch (v) {
        case 1:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECCKM);
            }
            break;
        case 2:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECANM);
            }else{
                resetFlag(flags, TF_KAM);
                notifyKAM();
            }
            break;
        case 3:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECCOLM);
                notifyDECCOLM();
            }
            break;
        case 4:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECSCLM);
                notifyDECSCLM();
            }else{
                resetFlag(flags, TF_IRM);
            }
            break;
        case 5:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECSCNM);
                notifyDECSCNM();
            }
            break;
        case 6:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECOM);
                notifyDECOM();
            }
            break;
        case 7:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECAWM);
            }
            break;
        case 8:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECARM);
            }
            break;
        case 12:
            if(!csi_data.decMode()) {
                resetFlag(flags, TF_SRM);
            }
            break;
        case 18:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECPFF);
            }
            break;
        case 19:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECPEX);
            }
            break;
        case 20:
            if(!csi_data.decMode()) {
                resetFlag(flags, TF_LNM);
            }
            break;
        case 25:
            if(csi_data.decMode()) {
                resetFlag(flags, TF_DECTCEM);
                notifyTCEM();
            }
            break;
        case 47:
        case 1047:
        case 1049:
            if(csi_data.decMode()) {               
                resetFlag(flags, TF_SCREEN);
                notifySCREEN();
                cursor = restoreCursor();
            }
            break;
        case 1048:
            cursor = restoreCursor();
            break;
        case 9: // SET_X10_MOUSE
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_9);
            }
            break;
        case 1000: //SET_VT200_MOUSE
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1000);
            }
            break;
        case 1001:  //SET_VT200_HIGHLIGHT_MOUSE
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1001);
            }
            break;
        case 1002:  //SET_BTN_EVENT_MOUSE
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1002);
            }
            break;
        case 1003: //SET_ANY_EVENT_MOUSE
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1003);
            }
            break;
        case 1006: //xterm extended mouse
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1006);
            }
            break;
        case 1015: //urxvt extended mouse
            if(csi_data.decMode()) {
                resetFlag(flags, TF_MOUSE_1015);
            }
            break;
        default:
            //qDebug() << "resetMode:v=" << v;
            break;
        }
    }
}

void QVte::notifyKAM()
{
    flagLockKeyboard(hasFlag(flags, TF_KAM));
}

void QVte::notifyKPAM()
{
    bool state = hasFlag(flags, TF_DECKPAM);
    flagApplicationKeypad(state);
}

void QVte::notifyTCEM()
{
    hc();
    flagCursorVisible(hasFlag(flags, TF_DECTCEM));
    sc();
}

void QVte::notifyDECCOLM()
{
    hc();
    int requested_width = (hasFlag(flags, TF_DECCOLM) ? 132 : 80);
    if (requestWidth(requested_width)) {
        columns = requested_width;
    }
    top = 0;
    bottom = rows - 1;
    cursor.reset();
    clearRegion(0, 0, columns - 1, rows - 1, cursor);
    sc();
}

void QVte::notifyDECSCLM()
{
    flagSoftScroll(hasFlag(flags, TF_DECSCLM));
}

void QVte::notifyDECSCNM()
{
    flagScreenReversed(hasFlag(flags, TF_DECSCNM));
}

void QVte::notifyDECOM()
{
    moveCursorAbsolute(0, 0);
}

void QVte::notifySCREEN()
{
    bool state = hasFlag(flags, TF_SCREEN);
    flagApplicationScreen(state);
    cursor = cursorNow();
}

void QVte::process(wchar_t c)
{
    if(isDel(c)) {
        return;
    }
    if (mode != TM_String || !handleString(c)) {
        if (isControl(c)) {
            handleControl(c);
        } else {
            switch (mode)
            {
            case TM_ESC:
                handleEsc(c);
                break;
            case TM_CSI:
                handleCsi(c);
                break;
            case TM_Hash:
                handleHash(c);
                break;
            case TM_Charset:
                handleCharset(c);
                break;
            case TM_UTF8:
                handleUtf8(c);
                break;
            default:
                handleDefault(c);
                break;
            }
        }
    }
}
