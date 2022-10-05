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

#include "qkxscreen.h"
#include "qkxhistory.h"

#include <QDebug>
#include <QtGlobal>

QKxScreen::QKxScreen(QObject *parent)
    : QObject(parent)
    , m_rows(40)
    , m_columns(80)
    , m_cursorVisible(false)
    , m_screenReverse(false)
    , m_icapture(1)
{
}

QKxScreen::~QKxScreen()
{
    m_selection.clear();
}

QKxHistory *QKxScreen::history()
{
    return m_history;
}

void QKxScreen::setHistory(QKxHistory *p)
{
    m_history = p;
}

void QKxScreen::setHistoryFile(const QString &file)
{
    if(m_historyFile != nullptr) {
        delete m_historyFile;
    }
    m_historyFile = new QKxHistoryFile(file, this);
}

void QKxScreen::stopHistoryFile()
{
    if(m_historyFile != nullptr) {
        delete m_historyFile;
    }
}

void QKxScreen::cleanHistory()
{
    m_history->clear();
}

void QKxScreen::clearAll()
{
    m_history->clear();
    m_image.clear();
    clearSelection();
}

void QKxScreen::clearScreen()
{
    while(!m_image.isEmpty()){
        const TermLine& line = m_image.takeFirst();
        m_history->append(line);
        outputToHistoryFile(line);
    }
    clearSelection();
}

void QKxScreen::resize(int row, int column)
{
    if(m_rows == row && m_columns == column) {
        return;
    }
    int n = m_image.length() - row;
    if(n > 0) {
        for(int i = 0; i < n; i++) {
            if(!m_image.isEmpty()) {
                TermLine line = m_image.takeFirst();
                if(!line.cs.isEmpty()){
                    m_history->append(line);
                    outputToHistoryFile(line);
                }
            }
        }
        m_cursorNow.y -= n;
        m_cursorSave.y -= n;
    }/*else{
       // revert to no msg reback from history.
        n = -n;
        if(n > m_history->lineCount()) {
            n = m_history->lineCount();
        }
        for(int i = 0; i < n; i++) {
            TermLine line = m_history->takeLast();
            m_image.insert(0, line);
        }
        m_cursorNow.y += n;
        m_cursorSave.y += n;
    }*/

    m_rows = row;
    m_columns = column;
    m_tabs.resize(column);
}

TermLine QKxScreen::lineAt(int y) const
{
    if(y < 0) {
        return TermLine();
    }
    if(m_history->lineCount() > y) {
        return m_history->lineAt(y);
    }
    y -= m_history->lineCount();
    if(y >= m_image.length() || y < 0) {
        return TermLine();
    }
    return m_image.at(y);
}

TermCursor QKxScreen::cursor() const
{
    return m_cursorNow;
}

bool QKxScreen::visibleCursor() const
{
    return m_cursorVisible;
}

bool QKxScreen::screenReverse() const
{
    return m_screenReverse;
}

void QKxScreen::clearSelection()
{
    m_selection.clear();
}

QMap<QPoint, QPoint> QKxScreen::selectionToView() const
{
    return m_selection;
}

void QKxScreen::setSelectionToView(const QPoint &start, const QPoint &end)
{
    m_selection.clear();
    if(qMapLessThanKey(start, end)) {
        m_selection.insert(start, end);
    }else{
        m_selection.insert(end, start);
    }
}

void QKxScreen::setSelectionToView(const QMap<QPoint, QPoint> &find)
{
    m_selection = find;
}

void QKxScreen::clearCapture()
{
    m_capture.clear();
}

int QKxScreen::setCapture(const QPoint &pt)
{
    int id = m_icapture++;
    m_capture.insert(id, pt);
    return id;
}

bool QKxScreen::releaseCapture(int id, QPoint *pt)
{
    if(m_capture.contains(id)) {
        QPoint tmp = m_capture.take(id);
        if(pt) {
            *pt = tmp;
        }
        return true;
    }
    return false;
}

void QKxScreen::resetColor(int c)
{
    //qDebug() << "resetColor(c:" << c << ")";
}

void QKxScreen::defineColor(int c, const wchar_t *s)
{
    //qDebug() << "define_color(c:" << c << ",s:"<< s << ")";
}

void QKxScreen::defineCharset(int n, TermCharset cs)
{
    //qDebug() << "define_charset(n:" << n << ",cs:" << cs << ")";
}

void QKxScreen::selectCharset(int n)
{
    //qDebug() << "select_charset(n:" << n << ")";
}

void QKxScreen::showCursor(const TermCursor &tc)
{
    //qDebug() << "showCursor: Screen" << quint64(this) << "maxLine" << history()->maxLine();
    m_cursorVisible = true;
    updateCursor(tc);
}

void QKxScreen::hideCursor(const TermCursor &tc)
{
    //qDebug() << "hideCursor: Screen" << quint64(this) << "maxLine" << history()->maxLine();
    m_cursorVisible = false;
    updateCursor(tc);
}

void QKxScreen::clearTabs()
{
    m_tabs.resize(m_columns);
    m_tabs.fill(false, m_columns);
}

void QKxScreen::setTab(int x)
{
    m_tabs.setBit(x, true);
}

void QKxScreen::resetTab(int x)
{
    m_tabs.setBit(x, false);
}

int QKxScreen::getNextTab(int x, int n)
{
    if( n > 0) {
        n = n == 0 ? 1 : n;
        while(n > 0 && x < m_columns - 1) {
            x += 1;
            while((x < m_columns - 1) && !m_tabs.testBit(x)) {
                x++;
            }
            n--;
        }
        return x;
    }
    n = n == 0 ? 1 : -n;
    while((n > 0) && (x > 0)) {
        x--;
        while((x > 0) && !m_tabs[x]) {
            x--;
        }
        n--;
    }
    return x;
}

void QKxScreen::putReversedQuestionMark(const TermCursor &tc)
{
    Q_UNUSED(tc)
    //qDebug() << "putReversedQuestionMark(n:";
}

void QKxScreen::putChar(wchar_t c, int char_width, const TermCursor &tc)
{
    if(tc.y < 0 || tc.x < 0) {
        return;
    }
    if(m_image.length() <= tc.y) {
        m_image.resize(tc.y+1);
    }
    TermLine &line = m_image[tc.y];
    if(line.cs.length() < (tc.x + char_width)) {
        line.cs.resize(tc.x+char_width);
    }
    TermChar tmp;
    tmp.c = c;
    tmp.wrap = tc.cs == TermCursor::CS_WrapNext;
    tmp.count = char_width;
    tmp.attr = tc.attrs;
    tmp.graphic = tc.graphicCharset();
    line.cs[tc.x] = tmp;
    if(char_width > 1) {
        tmp.c = 0;
        tmp.count = 0;
        tmp.attr.reset();
        line.cs[tc.x+1] = tmp;
    }
}

void QKxScreen::clearRegion(int x1, int y1, int x2, int y2, const TermCursor &tc)
{
    if(m_image.length() <= y2) {
        m_image.resize(y2+1);
    }
    for(int y = y1; y <= y2; y++) {
        TermLine &line = m_image[y];
        if(line.cs.length() <= x2) {
            line.cs.resize(x2+1);
        }
        for(int x = x1; x <= x2; x++) {
            line.cs[x].reset();
            line.cs[x].attr = tc.attrs;
        }
    }
}

void QKxScreen::insertChars(int x, int n, const TermCursor &tc)
{
    if(m_image.length() <= tc.y) {
        m_image.resize(tc.y);
    }
    TermLine& line = m_image[tc.y];
    if(line.cs.length() <= x) {
        line.cs.resize(x+1);
    }
    TermChar tmp;
    for(int i = 0; i < n; i++){
        line.cs.insert(x, tmp);
    }
    if(line.cs.length() > m_columns) {
        line.cs.resize(m_columns);
    }
}

void QKxScreen::deleteChars(int x, int n, const TermCursor &tc)
{
    if(m_image.length() <= tc.y) {
        m_image.resize(tc.y);
    }
    TermLine& line = m_image[tc.y];
    if(line.cs.length() <= x) {
        line.cs.resize(x+1);
    }
    for(int i = 0; i < n; i++){
        if(line.cs.length() > x) {
            line.cs.removeAt(x);
        }
    }
}

void QKxScreen::scrollUp(int y1, int y2, int n)
{
    // full screen scroll
    if(m_image.length() <= y2) {
        m_image.resize(y2 + 1);
    }
    int cnt = 0;
    for(int i = 0; i < n; i++){
        if(m_image.length() <= y2) {
            m_image.append(TermLine());
        }else{
            m_image.insert(y2+1, TermLine());
        }
        const TermLine& line = m_image.takeAt(y1);
        cnt += m_history->append(line);
        outputToHistoryFile(line);
    }
    // fix selection    
    if(cnt > 0){
        if(!m_selection.isEmpty()) {
            QMap<QPoint, QPoint> tmp;
            for(QMap<QPoint, QPoint>::iterator it = m_selection.begin(); it != m_selection.end(); it++)
            {
                QPoint pt1 = it.key();
                pt1.setY(pt1.y() - cnt);
                QPoint pt2 = it.value();
                pt2.setY(pt2.y() - cnt);
                if(m_history->maxLine() == 0) {
                    if(pt1.y() < y1) {
                        continue;
                    }
                }
                if(pt2.y() < 0) {
                    continue;
                }
                tmp.insert(pt1, pt2);
            }
            tmp.swap(m_selection);
        }
        if(!m_capture.isEmpty()) {
            QMap<int, QPoint> tmp;
            for(QMap<int,QPoint>::iterator it = m_capture.begin(); it != m_capture.end(); it++) {
                QPoint pt = it.value();
                pt.setY(pt.y() - cnt);
                if(m_history->maxLine() == 0) {
                    if(pt.y() < y1) {
                        continue;
                    }
                }
                tmp.insert(it.key(), pt);
            }
            tmp.swap(m_capture);
        }
    }
}

void QKxScreen::scrollDown(int y1, int y2, int n)
{
    if(m_image.length() <= y2) {
        m_image.resize(y2 + 1);
    }
    if(y1 == 0 && y2 == m_rows - 1) {
        for(int i = 0; i < n; i++){
            m_image.removeLast();
            m_image.insert(0, TermLine());
        }
    }else{
        for(int i = 0; i < n; i++){
            m_image.removeAt(y2);
            m_image.insert(y1, TermLine());
        }
    }
    // fix selection
    if(m_history->maxLine() == 0) {
        if(!m_selection.isEmpty()) {
            QMap<QPoint, QPoint> tmp;
            for(QMap<QPoint, QPoint>::iterator it = m_selection.begin(); it != m_selection.end(); it++)
            {
                QPoint pt1 = it.key();
                pt1.setY(pt1.y() + n);
                QPoint pt2 = it.value();
                pt2.setY(pt2.y() + n);
                if(pt2.y() > y2) {
                    continue;
                }
                tmp.insert(pt1, pt2);
            }
            tmp.swap(m_selection);
        }
    }
}

void QKxScreen::setLineTextSize(int y, TermTextSize t)
{
    if(m_image.length() <= y) {
        m_image.resize(y+1);
    }
    m_image[y].t = t;
}

void QKxScreen::flagCursorVisible(bool state)
{
    m_cursorVisible = state;
}

void QKxScreen::flagScreenReversed(bool state)
{
    m_screenReverse = state;
}

void QKxScreen::saveCursor(const TermCursor &cursor)
{
    m_cursorSave = cursor;
}

TermCursor QKxScreen::restoreCursor() const
{
    return m_cursorSave;
}

void QKxScreen::updateCursor(const TermCursor &cursor)
{
    m_cursorNow = cursor;
}

TermCursor QKxScreen::cursorNow() const
{
    return m_cursorNow;
}

void QKxScreen::outputToHistoryFile(const TermLine &line)
{
    if(m_historyFile) {
        m_historyFile->append(line);
    }
}
