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

#include "qwolinenoise.h"
#include "qkxtermwidget.h"

enum KEY_ACTION{
    KEY_NULL = 0,	    /* NULL */
    CTRL_A = 1,         /* Ctrl+a */
    CTRL_B = 2,         /* Ctrl-b */
    CTRL_C = 3,         /* Ctrl-c */
    CTRL_D = 4,         /* Ctrl-d */
    CTRL_E = 5,         /* Ctrl-e */
    CTRL_F = 6,         /* Ctrl-f */
    CTRL_H = 8,         /* Ctrl-h */
    TAB = 9,            /* Tab */
    CTRL_K = 11,        /* Ctrl+k */
    CTRL_L = 12,        /* Ctrl+l */
    ENTER = 13,         /* Enter */
    CTRL_N = 14,        /* Ctrl-n */
    CTRL_P = 16,        /* Ctrl-p */
    CTRL_T = 20,        /* Ctrl-t */
    CTRL_U = 21,        /* Ctrl+u */
    CTRL_W = 23,        /* Ctrl+w */
    ESC = 27,           /* Escape */
    BACKSPACE = 8,    /* Backspace */
    DEL = 127           /* delete */
};

QWoLineNoise::QWoLineNoise(QKxTermWidget *term)
    : m_term(term)
    , m_prompt("$")
{
    reset();
}

QWoLineNoise::~QWoLineNoise()
{

}

void QWoLineNoise::parse(const QByteArray &buf)
{
    if(buf.length() == 1) {
        if(buf[0] == 9) {
            // tab.
            if(m_state.completes.isEmpty()) {
                m_state.completes = handleComplete(m_state.buf);
                if(m_state.completes.isEmpty()) {
                    return;
                }
                m_state.completeIndex = 0;
                m_state.oldpos = m_state.pos;
            }else{
                m_state.completeIndex = (m_state.completeIndex+1) % m_state.completes.length();
            }
            m_state.buf = m_state.completes[m_state.completeIndex];
            m_state.pos = m_state.buf.count();
            refreshLine();
        }else if(!m_state.completes.isEmpty() && buf[0] == 27) {
            m_state.completes.clear();
            if(m_state.oldpos >= 0) {
                m_state.pos = m_state.oldpos;
            }
            m_state.buf = m_state.buf.left(m_state.pos);
            refreshLine();
        } else {
            m_state.completes.clear();
            m_state.oldpos = -1;
            m_state.buf = m_state.buf.left(m_state.pos);
            normalParse(buf);
        }
    }else{
        normalParse(buf);
    }
}

void QWoLineNoise::normalParse(const QByteArray &buf)
{
    int iread = 0;
    while(iread < buf.count()) {
        char seq[3];
        char c = buf.at(iread++);
        switch(c) {
        case ENTER:    /* enter */
        {
            editMoveEnd();
            QByteArray line = m_state.buf;
            parseSequenceText("\r\n");
            handleCommand(line);
            reset();
            parseSequenceText(m_prompt);
            return;
        }
        case CTRL_C:     /* ctrl-c */
            parseSequenceText("^C\r\n");
            reset();
            parseSequenceText(m_prompt);
            return ;
        case BACKSPACE:   /* backspace */
            editBackspace();
            break;
        case DEL:     /* ctrl-h */
            editDelete();
            break;
        case CTRL_D:     /* ctrl-d, remove char at right of cursor, or if the
                            line is empty, act as end-of-file. */
            if (m_state.buf.length() > 0) {
                editDelete();
            }
            break;
        case CTRL_B:     /* ctrl-b */
            editMoveLeft();
            break;
        case CTRL_F:     /* ctrl-f */
            editMoveRight();
            break;
        case CTRL_P:    /* ctrl-p */
            editHistoryPrev();
            break;
        case CTRL_N:    /* ctrl-n */
            editHistoryNext();
            break;
        case ESC:    /* escape sequence */
            /* Read the next two bytes representing the escape sequence.
             * Use two calls to handle slow terminals returning the two
             * chars at different times. */
            if(iread >= buf.count()) {
                return;
            }
            seq[0] = buf[iread++];
            if(iread >= buf.count()) {
                return;
            }
            seq[1] = buf[iread++];
            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if(iread >= buf.count()) {
                        return;
                    }
                    seq[2] = buf[iread++];
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        case '3': /* Delete key. */
                            editDelete();
                            break;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': /* Up */
                        editHistoryPrev();
                        break;
                    case 'B': /* Down */
                        editHistoryNext();
                        break;
                    case 'C': /* Right */
                        editMoveRight();
                        break;
                    case 'D': /* Left */
                        editMoveLeft();
                        break;
                    case 'H': /* Home */
                        editMoveHome();
                        break;
                    case 'F': /* End*/
                        editMoveEnd();
                        break;
                    }
                }
            } else if (seq[0] == 'O') {
                /* ESC O sequences. */
                switch(seq[1]) {
                case 'H': /* Home */
                    editMoveHome();
                    break;
                case 'F': /* End*/
                    editMoveEnd();
                    break;
                }
            }
            break;
        case CTRL_U: /* Ctrl+u, delete the whole line. */
            reset();
            refreshLine();
            break;
        case CTRL_K: /* Ctrl+k, delete from current to end of line. */
            m_state.buf = m_state.buf.left(m_state.pos);
            refreshLine();
            break;
        case CTRL_A: /* Ctrl+a, go to the start of the line */
            editMoveHome();
            break;
        case CTRL_E: /* ctrl+e, go to the end of the line */
            editMoveEnd();
            break;
        case CTRL_L: /* ctrl+l, clear screen */
            clearScreen();
            refreshLine();
            break;
        default:
            if(!QChar::fromLatin1(c).isPrint()){
                return;
            }
            editInsert(c);
            break;
        }
    }
}

void QWoLineNoise::setPrompt(const QByteArray &prompt)
{
    m_prompt = prompt;
}

void QWoLineNoise::handleCommand(const QByteArray &line)
{
    Q_UNUSED(line);
}

QList<QByteArray> QWoLineNoise::handleComplete(const QByteArray &line)
{
    Q_UNUSED(line);
    return QList<QByteArray>();
}

QByteArray QWoLineNoise::handleShowHints(QByteArray &line, int *pclr, int *pbold)
{
    Q_UNUSED(line);
    Q_UNUSED(pclr);
    Q_UNUSED(pbold);
    return QByteArray();
}

void QWoLineNoise::parseSequenceText(const QByteArray &line)
{
    Q_UNUSED(line);
    //m_term->parseSequenceText(line);
}

int QWoLineNoise::column()
{
    //return m_term->screenColumnsCount();
    return 0;
}

void QWoLineNoise::editDelete()
{
    if (m_state.pos < m_state.buf.length()) {
         m_state.buf.remove(m_state.pos, 1);
         refreshLine();
    }
}

void QWoLineNoise::editHistoryPrev()
{

}

void QWoLineNoise::editHistoryNext()
{

}


void QWoLineNoise::editMoveRight()
{
    if (m_state.pos != m_state.buf.length()) {
        m_state.pos++;
        refreshLine();
    }
}

void QWoLineNoise::editMoveLeft()
{
    if (m_state.pos > 0) {
        m_state.pos--;
        refreshLine();
    }
}

void QWoLineNoise::editMoveHome()
{
    if (m_state.pos != 0) {
        m_state.pos = 0;
        refreshLine();
    }
}

void QWoLineNoise::editBackspace()
{
    if (m_state.pos > 0 && !m_state.buf.isEmpty()) {
        m_state.buf.remove(m_state.pos-1, 1);
        m_state.pos--;
        refreshLine();
    }
}

void QWoLineNoise::clearScreen()
{
    parseSequenceText("\x1b[H\x1b[2J");
}

void QWoLineNoise::editDeletePrevWord()
{

}

void QWoLineNoise::beep()
{

}

/* Move cursor to the end of the line. */
void QWoLineNoise::editMoveEnd()
{
    if (m_state.pos != m_state.buf.length()) {
        m_state.pos = m_state.buf.length();
        refreshLine();
    }
}

void QWoLineNoise::reset()
{
    m_state.buf.resize(0);
    m_state.oldpos = -1;
    m_state.pos = 0;
    m_state.maxrows = 0;
}

void QWoLineNoise::editInsert(char c)
{
    if (m_state.buf.length() == m_state.pos) {
        m_state.buf.append(c);
    } else {
        m_state.buf.insert(m_state.pos, c);
    }
    m_state.pos++;
    refreshLine();

}

void QWoLineNoise::resetInput()
{
    QByteArray buf;
    buf.append("\033[0m");
    parseSequenceText(buf);
    refreshLine();
}

void QWoLineNoise::refreshLine()
{
    char seq[64];
    int plen = m_prompt.length();
    int rows = (plen+m_state.buf.length()+column()-1)/column(); /* rows used by current buf. */
    int rpos = (plen+m_state.oldpos+column())/column(); /* cursor relative row. */
    int old_rows = m_state.maxrows;
    QByteArray ab;

    /* Update maxrows if needed. */
    if (rows > m_state.maxrows) {
        m_state.maxrows = rows;
    }
    if (old_rows-rpos > 0) {
        int n = qsnprintf(seq,64,"\x1b[%dB", old_rows-rpos);
        ab.append(seq, n);
    }

    /* Now for every row clear it, go up. */
    for (int i = 0; i < old_rows-1; i++) {
        int n = qsnprintf(seq,64,"\r\x1b[0K\x1b[1A");
        ab.append(seq, n);
    }
    /* Clean the top line. */
    int n = qsnprintf(seq,64,"\r\x1b[0K");
    ab.append(seq, n);
    /* Write the prompt and the current buffer content */
    ab.append(m_prompt);
    ab.append(m_state.buf);

    /* Show hits if any. */
    ab.append(refreshShowHints());

    /* If we are at the very end of the screen with our prompt, we need to
         * emit a newline and move the prompt to the first column. */
    if(m_state.pos > 0
            && m_state.pos == m_state.buf.length()
            && (m_state.pos + m_prompt.length()) % column() == 0) {
        ab.append('\n');
        n = qsnprintf(seq, 64, "\r");
        ab.append(seq, n);
        rows++;
        if (rows > m_state.maxrows) {
            m_state.maxrows = rows;
        }
    }
    /* Move cursor to right position. */
    int rpos2 = (m_prompt.length()+m_state.pos+column())/column(); /* current cursor relative row. */
    /* Go up till we reach the expected positon. */
    if (rows-rpos2 > 0) {
        n = qsnprintf(seq,64,"\x1b[%dA", rows-rpos2);
        ab.append(seq, n);
    }
    /* Set column. */
    int col = (m_prompt.length()+m_state.pos) % column();
    if (col){
        n = qsnprintf(seq,64,"\r\x1b[%dC", col);
    } else {
        n = qsnprintf(seq,64,"\r");
    }
    ab.append(seq, n);    
    parseSequenceText(ab);
}

QByteArray QWoLineNoise::refreshShowHints()
{
    char seq[64];
    QByteArray ab;
    if (m_prompt.length()+m_state.buf.length() < column()) {
        int color = -1;
        int bold = 0;
        QByteArray hint = handleShowHints(m_state.buf, &color, &bold);
        if (!hint.isEmpty()) {
            int hintlen = hint.length();
            int hintmaxlen = column() - (m_prompt.length()+m_state.buf.length());
            if (hintlen > hintmaxlen){
                hintlen = hintmaxlen;
            }
            hint.resize(hintlen);
            if (bold && color == -1) {
                color = 8;
            }
            if (color != -1 || bold != 0){
                int n = qsnprintf(seq,64,"\033[%d;%d;49m", bold, color);
                ab.append(seq, n);
            }else{
                seq[0] = '\0';
            }
            ab.append(hint);
            if (color != -1 || bold != 0){
                ab.append("\033[0m");
            }
        }
    }
    return ab;
}
