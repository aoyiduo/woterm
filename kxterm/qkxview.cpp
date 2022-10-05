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

#include "qkxview.h"
#include "qkxscreen.h"
#include "qkxhistory.h"

#include <QDebug>

QKxView::QKxView(QObject *parent)
    : QObject(parent)
{

}

QKxView::~QKxView() {

}

void QKxView::setScreen(QKxScreen *p)
{
    m_screen = p;
    m_selectedText.clear();
    m_findKey = 0;
    m_findText.clear();
    m_screen->clearSelection();
}

QList<TermLine> QKxView::viewport(int y, int rows, int cols, bool *reverse, SelectionMode m)
{    
    if(m_screen == nullptr) {
        return QList<TermLine>();
    }
    if(reverse) {
        *reverse = m_screen->screenReverse();
    }
    QList<TermLine> img = copyImage(y, rows, cols);
    markSelection(img, y, rows, cols, m);
    return img;
}

int QKxView::lineCount() const
{
    QKxHistory *history = m_screen->history();
    return history->lineCount() + m_screen->rows();
}

int QKxView::historyLineCount() const
{
    QKxHistory *history = m_screen->history();
    return history->lineCount();
}

TermCursor QKxView::cursor() const
{
    return m_screen->cursor();
}

QPoint QKxView::cursorToScreenPosition() const
{
    TermCursor cursor = m_screen->cursor();
    return QPoint(cursor.x, cursor.y);
}

QPoint QKxView::cursorToViewPosition() const
{
    QKxHistory *history = m_screen->history();
    TermCursor cursor = m_screen->cursor();
    cursor.y += history->lineCount();
    return QPoint(cursor.x, cursor.y);
}

QPoint QKxView::viewToScreenPosition(const QPoint &pt) const
{
    QKxHistory *history = m_screen->history();
    return QPoint(pt.x(), pt.y() - history->lineCount());
}

void QKxView::clearSelection()
{
    m_screen->clearSelection();
    m_selectedText.clear();
    emit selectChanged();
}

QMap<QPoint,QPoint> QKxView::selection()
{  
    return m_screen->selectionToView();
}

void QKxView::setSelection(QPoint start, QPoint end)
{
    m_screen->setSelectionToView(start, end);
    m_selectedText.clear();
    emit selectChanged();
}

void QKxView::setSelection(const QMap<QPoint, QPoint> &sels)
{
    m_screen->setSelectionToView(sels);
    m_selectedText.clear();
    emit selectChanged();
}

void QKxView::selectWord(QPoint pt)
{
    int cols = m_screen->columens();
    int x = pt.x();
    int y = pt.y();
    const TermLine& line = lineAt(pt.y());
    if(line.cs.isEmpty()) {
        setSelection(QPoint(0, y), QPoint(cols-1, y));
    }else if(line.cs.length() <= x) {
        setSelection(QPoint(line.cs.length(), y), QPoint(cols - 1, y));
    }else{
        int x1 = x;
        QChar t = typeCheck(line.cs.at(x1).c);
        while(x1 >= 0) {
            QChar hit = typeCheck(line.cs.at(x1).c);
            if(hit != t) {
                x1++;
                break;
            }
            x1--;
        }
        if(x1 < 0) {
            x1 = 0;
        }
        int x2 = x;
        while(x2 < line.cs.length()) {
            QChar hit = typeCheck(line.cs.at(x2).c);
            if(hit != t) {
                x2--;
                break;
            }
            x2++;
        }
        if(x2 == line.cs.length()) {
            x2--;
        }
        setSelection(QPoint(x1, y), QPoint(x2, y));
    }
}

void QKxView::selectLine(int y)
{
    int rows = lineCount();
    int cols = m_screen->columens();
    const TermLine& line = lineAt(y);
    if(line.cs.isEmpty()) {
        setSelection(QPoint(0, y), QPoint(cols, y));
    }else {
        if(line.cs.at(0).wrap) {
            int y1 = y;
            y1--;
            while(y1 >= 0) {
                const TermLine& line = lineAt(y1);
                if(line.cs.isEmpty()) {
                    y1++;
                    break;
                }
                if(!line.cs.at(0).wrap) {
                    break;
                }
                y1--;
            }
            if(y1 < 0) {
                y1 = 0;
            }
            int y2 = y;
            y2++;
            while(y2 < rows) {
                const TermLine& line = lineAt(y2);
                if(line.cs.isEmpty()) {
                    y2--;
                    setSelection(QPoint(0, y1), QPoint(cols-1, y2));
                    return ;
                }
                if(!line.cs.at(0).wrap) {
                    y2--;
                    setSelection(QPoint(0, y1), QPoint(cols-1, y2));
                    return ;
                }
                y2++;
            }
            setSelection(QPoint(0, y1), QPoint(cols-1, rows - 1));
        }else{
            int y1 = y;
            int y2 = y;
            y2++;
            while(y2 < rows) {
                const TermLine& line = lineAt(y2);
                if(line.cs.isEmpty()) {
                    y2--;
                    setSelection(QPoint(0, y1), QPoint(cols-1, y2));
                    return;
                }
                if(!line.cs.at(0).wrap) {
                    y2--;
                    setSelection(QPoint(0, y1), QPoint(cols-1, y2));
                    return;
                }
                y2++;
            }
            setSelection(QPoint(0, y1), QPoint(cols-1, rows - 1));
        }
    }
}

TermLine QKxView::lineAt(int y) const
{
    return m_screen->lineAt(y);
}

QString QKxView::plainText(const QPoint &start, const QPoint &end, SelectionMode m) const
{
    QPoint pt1 = start;
    QPoint pt2 = end;
    QString out;
    if(m == SM_RectText) {
        return out;
    }
    if(pt1.y() == pt2.y()) {
        TermLine line = lineAt(pt1.y());
        int rx = 0;
        for(int x = 0; x < line.cs.length(); x++) {
            const TermChar& c = line.cs.at(x);
            rx += c.count;
            if(rx - 1 >= pt1.x() && rx - 1 <= pt2.x() && c.c != 0) {
                out.append(c.c);
            }
        }
    }else {
        for(int y = pt1.y(); y <= pt2.y(); y++) {
            TermLine line = lineAt(y);
            if(line.cs.isEmpty() || !line.cs.at(0).wrap) {
                if(y != pt1.y()) {
                    int i = 0;
                    for(i = out.length() - 1; i >= 0; i--) {
                        if(out.at(i) != QChar::Space) {
                            out.resize(i+1);
                            break;
                        }
                    }
                    if(i < 0) {
                        out.resize(0);
                    }
                    out.append("\n");
                }
            }
            if(line.cs.isEmpty()) {
                continue;
            }
            if(y == pt1.y()) {
                int rx = 0;
                for(int x = 0; x < line.cs.length(); x++) {
                    const TermChar& c = line.cs.at(x);
                    rx += c.count;
                    if(rx - 1 >= pt1.x() && c.c != 0) {
                        out.append(c.c);
                    }
                }
            }else if(y == pt2.y()) {
                int rx = 0;
                for(int x = 0; x < line.cs.length(); x++) {
                    const TermChar& c = line.cs.at(x);
                    rx += c.count;
                    if(rx - 1 <= pt2.x() && c.c != 0) {
                        out.append(c.c);
                    }
                }
            }else {
                for(int x = 0; x < line.cs.length(); x++) {
                    const TermChar& c = line.cs.at(x);
                    if(c.c != 0){
                        out.append(c.c);
                    }
                }
            }
            if(line.t == TS_TopHalf || line.t == TS_BottomHalf) {
                y++;
            }
        }
    }
    int i = 0;
    for(i = out.length() - 1; i >= 0; i--) {
        if(out.at(i) != QChar::Space) {
            out.resize(i+1);
            break;
        }
    }
    if(i < 0) {
        out.resize(0);
    }
    return out;
}

QString QKxView::selectedText()
{
    if(!m_selectedText.isEmpty()) {
        return m_selectedText;
    }
    QMap<QPoint,QPoint> tmp = selection();
    if(tmp.isEmpty()) {
        return "";
    }
    m_selectedText = plainText(tmp.firstKey(), tmp.first(), m_mode);
    return m_selectedText;
}

QList<TermLine> QKxView::selectedLines()
{
    return QList<TermLine>();
}

bool QKxView::find(const QString &key, bool match, bool regular)
{
    QList<int> pos;
    pos.reserve(m_screen->columens());
    m_findText = key;
    m_findLength = 0;
    if(key.isEmpty()) {
        clearSelection();
        return true;
    }    
    Qt::CaseSensitivity sensitive = match ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp::PatternSyntax syntax = regular ? QRegExp::RegExp : QRegExp::FixedString;
    QRegExp rgx(key, sensitive, syntax);
    QPoint pt1 = intToPoint(m_findKey);
    for(int y = pt1.y(); y < lineCount(); y++) {
        pos.clear();
        QString line;
        if(!lineText(y, line, pos)) {
            continue;
        }
        int idx = 0;
        if(y == pt1.y()) {
            idx = pt1.x();
        }
        idx = rgx.indexIn(line, idx);
        if(idx >= 0) {
            const QStringList& caps = rgx.capturedTexts();            
            int cnt = caps.at(0).length();
            QPoint pt1 = QPoint(pos.at(idx), y);
            QPoint pt2 = QPoint(cnt - 1 + idx, y);
            m_findKey = intFromPoint(pt1);
            m_findLength = cnt;
            setSelection(pt1, pt2);
            return true;
        }
    }
    m_findKey = 0;
    clearSelection();
    return false;
}

bool QKxView::findPrev(bool match, bool regular)
{
    QList<int> pos;
    pos.reserve(m_screen->columens());
    if(m_findText.isEmpty()) {
        return true;
    }
    Qt::CaseSensitivity sensitive = match ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp::PatternSyntax syntax = regular ? QRegExp::RegExp : QRegExp::FixedString;
    QRegExp rgx(m_findText, sensitive, syntax);
    QPoint pt1 = intToPoint(m_findKey);
    for(int y = pt1.y(); y >= 0; y--) {
        pos.clear();
        QString line;
        if(!lineText(y, line, pos)) {
            continue;
        }
        int idx = line.length();
        if(y == pt1.y()) {
            idx = qMin(pt1.x(), line.length());
            line.resize(idx);
        }
        idx = rgx.lastIndexIn(line, idx);
        if(idx >= 0){
            const QStringList& caps = rgx.capturedTexts();
            int cnt = caps.at(0).length();
            QPoint pt1 = QPoint(pos.at(idx), y);
            QPoint pt2 = QPoint(cnt - 1 + idx, y);
            m_findKey = intFromPoint(pt1);
            m_findLength = cnt;
            setSelection(pt1, pt2);
            return true;
        }
    }
    m_findKey = intFromPoint(QPoint(10000,lineCount()));
    return findPrev(match, regular);
}

bool QKxView::findNext(bool match, bool regular)
{
    QList<int> pos;
    pos.reserve(m_screen->columens());
    if(m_findText.isEmpty()) {
        return true;
    }

    Qt::CaseSensitivity sensitive = match ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp::PatternSyntax syntax = regular ? QRegExp::RegExp : QRegExp::FixedString;
    QRegExp rgx(m_findText, sensitive, syntax);
    QPoint pt1 = intToPoint(m_findKey);
    for(int y = pt1.y(); y < lineCount(); y++) {
        pos.clear();
        QString line;
        if(!lineText(y, line, pos)) {
            continue;
        }
        int idx = 0;
        if(y == pt1.y()) {
            idx = pt1.x() + m_findLength;
        }
        idx = rgx.indexIn(line, idx);
        if(idx >= 0) {
            const QStringList& caps = rgx.capturedTexts();
            int cnt = caps.at(0).length();
            QPoint pt1 = QPoint(pos.at(idx), y);
            QPoint pt2 = QPoint(cnt - 1 + idx, y);
            m_findKey = intFromPoint(pt1);
            m_findLength = cnt;
            setSelection(pt1, pt2);
            return true;
        }
    }
    m_findKey = 0;
    return findNext(match, regular);
}

void QKxView::findAll(bool match, bool regular)
{
    QList<int> pos;
    pos.reserve(m_screen->columens());
    clearSelection();
    if(m_findText.isEmpty()) {
        return;
    }
    QMap<QPoint, QPoint> sels;
    Qt::CaseSensitivity sensitive = match ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp::PatternSyntax syntax = regular ? QRegExp::RegExp : QRegExp::FixedString;
    QRegExp rgx(m_findText, sensitive, syntax);
    for(int y = 0; y < lineCount(); y++) {
        pos.clear();
        QString line;
        if(!lineText(y, line, pos)) {
            continue;
        }
        int idx = 0;
        while ((idx = rgx.indexIn(line, idx)) != -1) {
            const QStringList& caps = rgx.capturedTexts();
            for(int i = 0; i < caps.length(); i++) {
                int cnt = caps.at(i).length();
                sels.insert(QPoint(pos.at(idx), y), QPoint(cnt - 1 + idx, y));
                idx += cnt;
            }
        }
    }
    setSelection(sels);
}

void QKxView::clearCapture()
{
    m_screen->clearCapture();
}

int QKxView::setCapture(const QPoint &pt)
{
    return m_screen->setCapture(pt);
}

bool QKxView::releaseCapture(int id, QPoint *pt)
{
    return m_screen->releaseCapture(id, pt);
}

QList<TermLine> QKxView::copyImage(int y, int rows, int cols)
{
    QList<TermLine> img;
    int hiscnt = historyLineCount();
    const TermCursor& tc = m_screen->cursor();
    bool visible = m_screen->visibleCursor();
    int tcy = tc.y + hiscnt;
    for(int r = y; r < rows + y; r++) {
        const TermLine &vtc = lineAt(r);
        img.append(vtc);
        if(vtc.cs.length() > cols) {
            img.last().cs.resize(cols);
        }
        TermLine &last = img.last();
        if(r == tcy && visible) {            
            last.xcur = tc.x;
            if(last.cs.length() < tc.x - 2) {
                int i = tc.x - 2;
            }
            while(last.cs.length() <= tc.x) {
                last.cs.append(TermChar());
            }
        }
    }
    return img;
}

void QKxView::markSelection(QList<TermLine> &img, int y, int rows, int cols, SelectionMode m)
{
    QMap<QPoint,QPoint> sels = selection();
    if(sels.isEmpty()) {
        return;
    }
    for(QMap<QPoint,QPoint>::iterator iter = sels.begin(); iter != sels.end(); iter++) {
        QPoint pt1 = iter.key();
        QPoint pt2 = iter.value();
        pt1 = QPoint(pt1.x(), pt1.y() - y);
        pt2 = QPoint(pt2.x(), pt2.y() - y);
        bool interset = !(pt2.y() < 0 || pt1.y() > rows - 1);
        if(interset) {
            if(m == SM_RectText) {
                QRect sel = QRect(pt1, pt2).normalized().intersected(QRect(0, 0, cols-1, rows-1));
                for(int r = sel.top(); r <= sel.bottom(); r++) {
                    if(img.length() <= r) {
                        img.append(TermLine());
                    }
                    TermLine& line = img[r];
                    if(line.cs.length() <= sel.right()) {
                        line.cs.resize(sel.right()+1);
                    }
                    for(int c = sel.left(); c <= sel.right(); c++) {
                        TermAttributes& attr = line.cs[c].attr;
                        if(attr.flags & TermAttributes::AF_Reverse) {
                            attr.flags &= ~quint32(TermAttributes::AF_Reverse);
                        }else{
                            attr.flags |= TermAttributes::AF_Reverse;
                        }
                    }
                }
            }else if(m == SM_LineText){
                if(pt1.y() < 0) {
                    pt1.setX(0);
                }
                if(pt2.y() > rows - 1) {
                    pt2.setX(cols - 1);
                }
                pt1.setY(qBound(0, pt1.y(), rows - 1));
                pt1.setX(qBound(0, pt1.x(), cols - 1));
                pt2.setY(qBound(0, pt2.y(), rows - 1));
                pt2.setX(qBound(0, pt2.x(), cols - 1));
                for(int r = pt1.y(); r <= pt2.y(); r++) {
                    if(img.length() <= r) {
                        img.append(TermLine());
                    }
                    TermLine& line = img[r];
                    int cbegin = r == pt1.y() ? pt1.x() : 0;
                    int cend = r == pt2.y() ? pt2.x() : cols - 1;
                    if(line.cs.length() <= cend) {
                        line.cs.resize(cend+1);
                    }
                    for(int c = cbegin; c <= cend; c++) {
                        TermAttributes& attr = line.cs[c].attr;
                        if(attr.flags & TermAttributes::AF_Reverse) {
                            attr.flags &= ~quint32(TermAttributes::AF_Reverse);
                        }else{
                            attr.flags |= TermAttributes::AF_Reverse;
                        }
                    }
                }
            }
        }
    }
}

QChar QKxView::typeCheck(QChar c)
{
    if(c.isSpace()) {
        return QChar::Space;
    }
    if(c.isLetterOrNumber() || QString("@.-_").contains(c)) {
        return QChar('a');
    }
    return c;
}

bool QKxView::lineText(int y, QString& out, QList<int> &pos)
{
    const TermLine& line = lineAt(y);
    if(line.cs.isEmpty()) {
        pos.clear();
        out.clear();
        return false;
    }
    for(int x = 0; x < line.cs.length(); x++) {
        const TermChar& c = line.cs.at(x);
        if(c.c != 0) {
            pos.append(x);
            out.push_back(c.c);
        }
    }
    return true;
}
