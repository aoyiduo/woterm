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

#ifndef QVTEDEF_H
#define QVTEDEF_H

#include "qkxterm_share.h"

#include <QtGlobal>
#include <QVector>
#include <QPoint>

#define DEFAULT_FONT_FAMILY "Courier New"
#define DEFAULT_KEY_LAYOUT  "default"
#define DEFAULT_TEXT_CODEC  "UTF-8"
#define DEFAULT_FONT_SIZE (13)
#define DEFAULT_COLOR_SCHEMA  "Ubuntu"

#define INVALID_POINT_VALUE (-0x7FFFFF)

#define REPCHAR   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgjijklmnopqrstuvwxyz0123456789./+@"
const QChar LTR_OVERRIDE_CHAR( 0x202D );

const int CharsetTableSize = 4;

enum TermCharset {
    CS_UK = 0,
    CS_US,
    CS_G0,
    CS_G1
};

enum TermTextSize {
    TS_Normal = 0,
    TS_TopHalf,
    TS_BottomHalf,
    TS_DoubleWidth
};

struct TermColor {
    enum Type {
        CT_Default = 0,
        CT_Indexed,
        CT_Truecolor
    };

    TermColor() { reset(); }

    inline bool isDefault() const {
        return type == CT_Default;
    }

    inline bool isColorful() {
        return type != CT_Default;
    }

    inline bool isIndex() const {
        return type == CT_Indexed;
    }

    void reset() {
        type = CT_Default;
        color = 0;
    }

    bool equal(const TermColor& c) {
        return type == c.type && color == c.color;
    }

    Type type;
    unsigned int color;
};

struct TermAttributes {
    enum AttributeFlags {
        AF_Underline  = (1 <<  0),
        AF_Reverse    = (1 <<  1),
        AF_Bold       = (1 <<  2),
        AF_Blink      = (1 <<  3),
        AF_HalfBright = (1 <<  4),
        AF_Italic     = (1 <<  5),
        AF_Invisible  = (1 <<  6),
        AF_Overline   = (1 <<  7)
    };

    TermAttributes() { reset(); }

    void reset() {
        fg.reset();
        bg.reset();
        flags = 0;
    }

    bool equal(const TermAttributes& o) {
        return fg.equal(o.fg) && bg.equal(o.bg) && flags == o.flags;
    }

    TermColor fg;
    TermColor bg;
    unsigned int flags;
};

struct TermCursor {
    enum State {
        CS_Normal = 0,
        CS_WrapNext
    };

    TermCursor() {
        reset();
    }

    void reset() {
        attrs.reset();
        x = y = 0;
        cs = CS_Normal;
        for (int i = 0; i < CharsetTableSize; ++i) {
            charset_table[i] = CS_US;
        }
        selected_charset = 0;
    }

    bool graphicCharset() const {
        return charset_table[selected_charset] == CS_G0;
    }

    TermCharset currentCharset() const {
        return charset_table[selected_charset];
    }

    int x;
    int y;
    TermAttributes attrs;
    State cs;
    TermCharset charset_table[CharsetTableSize];
    int selected_charset;
};

const int UTF8_BUF_LEN = 8;
const int STR_BUF_LEN = 32;

enum TermMode {
    TM_Ground = 0,
    TM_ESC,
    TM_CSI,
    TM_Hash,
    TM_Charset,
    TM_UTF8,
    TM_String
};

enum TermFlags {
    TF_KAM       = (1 <<  1),   // keyboard action mode
    TF_SRM   = (1 <<  0),       // send/receive mode
    TF_IRM     = (1 <<  2),     // insert/replace mode, none zero is insert mode, other is replace mode
    TF_LNM     = (1 <<  3),     // line feedback/new line mode
    TF_DECTCEM = (1 <<  4),     // text cursor enable mode
    TF_DECCKM  = (1 <<  5),     // cursor keys mode
    TF_DECANM  = (1 <<  6),     // ANSI/VT52 mode //Designate VT52 mode (DECANM)
    TF_DECCOLM = (1 <<  7),     // column mode (80/132)
    TF_DECSCLM = (1 <<  8),     // scolling mode (soft scroll)
    TF_DECSCNM = (1 <<  9),     // screen mode
    TF_DECOM   = (1 << 10),     // origin mode
    TF_DECAWM  = (1 << 11),     // autowrap mode
    TF_DECARM  = (1 << 12),     // auto repeat mode
    TF_DECPFF  = (1 << 13),     // print form feed mode
    TF_DECPEX  = (1 << 14),     // print extent mode
    TF_DECKPAM = (1 << 15),     // keypad application mode
    TF_UTF8    = (1 << 16),      // UTF8 mode
    TF_SCREEN  = (1 << 17),      // AppScreen mode
    TF_MOUSE_9 = (1 << 18),      // SET_X10_MOUSE
    TF_MOUSE_1000 = (1 << 19),   // SET_VT200_MOUSE
    TF_MOUSE_1001 = (1 << 20),  // SET_VT200_HIGHLIGHT_MOUSE
    TF_MOUSE_1002 = (1 << 21),  // SET_BTN_EVENT_MOUSE
    TF_MOUSE_1003 = (1 << 22),  // SET_ANY_EVENT_MOUSE
    TF_MOUSE_1006 = (1 << 23),  // xterm extended mouse
    TF_MOUSE_1015 = (1 << 24)   // urxvt extended mouse
};

typedef struct TermChar {
    TermAttributes attr;
    wchar_t c;
    bool wrap;
    bool graphic;
    int count;

    TermChar() {
        reset();
    }

    inline TermColor color() const {
        return attr.fg;
    }

    inline TermColor backColor() const {
        return attr.bg;
    }

    inline bool isGraphic() const {
        return graphic;
    }

    inline bool isDefaultColor(bool bfg) {
        return bfg ? attr.fg.isDefault() : attr.bg.isDefault();
    }

    inline bool isReverse() const {
        return attr.flags & TermAttributes::AF_Reverse;
    }

    inline bool isBold() const {
        return attr.flags & TermAttributes::AF_Bold;
    }

    inline bool isBlink() const {
        return attr.flags & TermAttributes::AF_Blink;
    }

    inline bool isHalfBright() const {
        return attr.flags & TermAttributes::AF_HalfBright;
    }

    inline bool isItalic() const {
        return attr.flags & TermAttributes::AF_Italic;
    }

    inline bool isInvisible() const {
        return attr.flags & TermAttributes::AF_Invisible;
    }

    inline bool isOverline() const {
        return attr.flags & TermAttributes::AF_Overline;
    }

    inline bool isUnderline() const {
        return attr.flags & TermAttributes::AF_Underline;
    }

    inline bool equal(const TermChar& o) {
        return c == o.c && graphic == o.graphic && attr.equal(o.attr);
    }

    void reset() {
        c = 32;
        count = 1;
        wrap = false;
        graphic = false;
        attr.reset();
    }
}TermChar;

typedef struct TermLine {
    TermTextSize t;
    QVector<TermChar> cs;
    int xcur;

    TermLine() {
        t = TS_Normal;
        xcur = -1;
        cs.reserve(50);
    }
} TermLine;

enum SelectionMode {
    SM_RectText = 1,
    SM_LineText = 2
};


inline int intFromPoint(const QPoint& pt) {
    return pt.y() << 14 | pt.x();
}

inline QPoint intToPoint(int v) {
    return QPoint(v & 0x3FFF, v >> 14);
}

inline bool qMapLessThanKey(const QPoint &key1, const QPoint &key2)
{
    int k1 = intFromPoint(key1);
    int k2 = intFromPoint(key2);
    return k1 < k2;
}

#endif // QVTEDEF_H
