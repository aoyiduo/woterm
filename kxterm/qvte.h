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

#ifndef QVTE_H
#define QVTE_H

#include "qvtedef.h"


class QVte
{
public:
    QVte();
    virtual ~QVte();
    void process(const wchar_t *buf, int length);
    void resize(int rows, int cols);
    void reset();
    unsigned int states();
protected:
    // to implement
    virtual void screenResize(int rows, int cols) = 0;
    virtual void resetColor(int c) = 0;
    virtual void defineColor(int c, const wchar_t *s) = 0;
    virtual void defineCharset(int n, TermCharset cs) = 0;
    virtual void selectCharset(int n) = 0;
    virtual void showCursor(const TermCursor& cursor) = 0;
    virtual void hideCursor(const TermCursor& cursor) = 0;
    virtual void clearTabs() = 0;
    virtual void setTab(int x) = 0;
    virtual void resetTab(int x) = 0;
    virtual int getNextTab(int x, int n) = 0;
    virtual int getCharWidth(wchar_t c, const TermCursor& cursor) = 0;
    virtual void putReversedQuestionMark(const TermCursor& cursor) = 0;
    virtual void putChar(wchar_t c, int char_width, const TermCursor& cursor) = 0;
    virtual bool requestWidth(int width) = 0;
    virtual void clearRegion(int x1, int y1, int x2, int y2, const TermCursor& cursor) = 0;
    virtual void insertChars(int x, int n, const TermCursor& cursor) = 0;
    virtual void deleteChars(int x, int n, const TermCursor& cursor) = 0;
    virtual void scrollUp(int y1, int y2, int n) = 0;
    virtual void scrollDown(int y1, int y2, int n) = 0;
    virtual void bell() = 0;
    virtual void setLineTextSize(int y, TermTextSize t) = 0;
    virtual void ttyWrite(const char *s) = 0;
    virtual void setWindowTitle(const wchar_t *title) = 0;
    virtual void flagLockKeyboard(bool state) = 0;
    virtual void flagApplicationScreen(bool state) = 0;
    virtual void flagApplicationKeypad(bool state) = 0;
    virtual void flagCursorVisible(bool state) = 0;
    virtual void flagSoftScroll(bool state) = 0;
    virtual void flagScreenReversed(bool state) = 0;
    virtual void saveCursor(const TermCursor& cursor) = 0;
    virtual TermCursor restoreCursor() const = 0;
    virtual void updateCursor(const TermCursor& cursor) = 0;
    virtual TermCursor cursorNow() const = 0;
private:
    void RIS();
    void LFVT();
    bool NEL();
    void IND();
    void RI();
    void DECID();
    void DECSC();
    void DECRC();
    void CPR();
    void DECALN();

    bool handleString(wchar_t c);
    void handleControl(wchar_t c);
    void handleEsc(wchar_t c);
    void handleCsi(wchar_t c);
    void handleHash(wchar_t c);
    void handleCharset(wchar_t c);
    void handleUtf8(wchar_t c);
    void handleDefault(wchar_t c);
    void parseString();
    void resetEscape();
    void hc();
    void sc();
    void setCharPre(int char_width);
    void setChar(wchar_t c, int char_width);
    void setCharPost(int char_width);
    void moveCursor(int x, int y);
    void moveCursorAbsolute(int x, int y);
    void insertLines(int y, int n);
    void deleteLines(int y, int n);
    void trmInsertChars(int x, int n);
    void trmDeleteChars(int x, int n);
    void trmClearRegion(int x1, int y1, int x2, int y2);
    void trmClearEntireLine();
    void trmClearToEndOfLine();
    void trmClearToBeginOfLine();
    void trmClearToEndOfScreen();
    void trmClearToBeginOfScreen();
    void trmClearEntireScreen();
    void trmClearHistory();
    void trmScrollUp(int y, int n);
    void trmScrollDown(int y, int n);
    void setColor(int& n, TermColor& color);
    void setAttributes();
    void restoreModes();
    void saveModes();
    void setModes();
    void resetModes();
    void notifyKAM();
    void notifyKPAM();
    void notifyTCEM();
    void notifyDECCOLM();
    void notifyDECSCLM();
    void notifyDECSCNM();
    void notifyDECOM();
    void notifySCREEN();

private:
    void process(wchar_t c);
private:
    struct ControlData
    {
        ControlData(){
            reset();
        }

        void reset() {
            for(int i = 0; i < CTRL_DATA_LEN; i++) {
                argv[i] = 0;
            }
            argc = 1;
            dec_mode = false;
        }

        inline int getArg(int i) {
            if (i >= 0 && i < argc) {
                return argv[i];
            }
            return 0;
        }

        inline bool spaceLeft(){
            return argc < CTRL_DATA_LEN;
        }

        inline bool decMode() {
            return dec_mode;
        }

        bool addArg(wchar_t c){
            if (c == '?' && argc == 1) {
                dec_mode = true;
                return true;
            } else if (c == ';') {
                argc++;
                return true;
            } else if (c >= '0' && c <= '9') {
                argv[argc-1] = 10*argv[argc-1] + c - '0';
                return true;
            }
            return false;
        }

        static const int CTRL_DATA_LEN = 32;
        int argv[CTRL_DATA_LEN];
        int argc;
        bool dec_mode;
    };

    struct StringData {
        enum StringType {
            ST_None = 0,
            ST_Title,
            ST_OSC,
            ST_PM,
            ST_DCS,
            ST_APC
        };

        StringData() {
            reset();
        }

        inline void reset() {
            reset(ST_None);
        }

        void reset(StringType st) {
            for(int i = 0; i < STRING_DATA_LEN; i++) {
                str_data[i] = 0;
            }
            next_str_data = str_data;
            string_type = st;
            finished = false;
            nargs = 1;
            args[0] = str_data;
        }

        int parse(wchar_t *s, int n) {
            int i = 0;
            while (*s && i < n && nargs < STRING_ARG_LEN) {
                if (*s == ';') {
                    *s = 0;
                    i++;
                    args[nargs++] = s + 1;
                }
                s++;
            }
            return i;
        }

        inline StringType type(){
            return string_type;
        }

        inline bool isFinished() {
            return finished;
        }

        inline void setFinished() {
            finished = true;
        }

        inline wchar_t *data() {
            return str_data;
        }

        inline wchar_t *getArg(int arg) {
            if (arg >= 0 && arg < nargs) {
                return args[arg];
            }
            return nullptr;
        }

        inline bool addChar(wchar_t c) {
            if (next_str_data >= str_data + STRING_DATA_LEN - 2) {
                return false;
            }
            *next_str_data++ = c;
            return true;
        }

        static const int STRING_DATA_LEN = 256;
        static const int STRING_ARG_LEN = 32;
        wchar_t str_data[STRING_DATA_LEN];
        wchar_t *next_str_data;
        StringType string_type;
        bool finished;
        wchar_t *args[STRING_ARG_LEN];
        int nargs;
    };
private:
    int rows;
    int columns;
    int top;
    int bottom;
    int defining_charset;
    TermCursor cursor;
    TermMode mode;
    bool cursor_hidden;
    unsigned int flags;
    ControlData csi_data;
    StringData str_data;
};

#endif // QVTE_H
