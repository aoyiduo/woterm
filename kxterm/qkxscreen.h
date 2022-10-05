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

#ifndef QSCREEN_H
#define QSCREEN_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QVector>
#include <QBitArray>
#include <QMap>

#include "qvtedef.h"

class QKxHistory;
class QVteImpl;

class QKxScreen : public QObject
{
    Q_OBJECT
public:
    explicit QKxScreen(QObject *parent = nullptr);
    ~QKxScreen();

    QKxHistory *history();
    void setHistory(QKxHistory* p);
    void setHistoryFile(const QString& file);
    void stopHistoryFile();
    void cleanHistory();
    void clearAll();
    void clearScreen();
    void resize(int rows, int cols);
    inline int rows() { return m_rows; }
    inline int columens() { return m_columns; }
    TermLine lineAt(int y) const;
    TermCursor cursor() const;
    bool visibleCursor() const;
    bool screenReverse() const;
    void clearSelection();
    QMap<QPoint,QPoint> selectionToView() const;
    void setSelectionToView(const QPoint& start, const QPoint& end);
    void setSelectionToView(const QMap<QPoint,QPoint>& find);

    // capture point.
    void clearCapture();
    int setCapture(const QPoint& pt);
    bool releaseCapture(int id, QPoint* pt);
protected:
    virtual void resetColor(int c);
    virtual void defineColor(int c, const wchar_t *s);
    virtual void defineCharset(int n, TermCharset cs);
    virtual void selectCharset(int n);
    virtual void showCursor(const TermCursor& cursor);
    virtual void hideCursor(const TermCursor& cursor);
    virtual void clearTabs();
    virtual void setTab(int x);
    virtual void resetTab(int x);
    virtual int getNextTab(int x, int n);
    virtual void putReversedQuestionMark(const TermCursor& tc);
    virtual void putChar(wchar_t c, int char_width, const TermCursor& tc);
    virtual void clearRegion(int x1, int y1, int x2, int y2, const TermCursor& tc);
    virtual void insertChars(int x, int n, const TermCursor& tc);
    virtual void deleteChars(int x, int n, const TermCursor& tc);
    virtual void scrollUp(int y1, int y2, int n);
    virtual void scrollDown(int y1, int y2, int n);
    virtual void setLineTextSize(int y, TermTextSize t);
    virtual void flagCursorVisible(bool state);
    virtual void flagScreenReversed(bool state);
    virtual void saveCursor(const TermCursor& cursor);
    virtual TermCursor restoreCursor() const;
    virtual void updateCursor(const TermCursor& cursor);
    virtual TermCursor cursorNow() const;
private:
    void outputToHistoryFile(const TermLine& line);
private:
    friend class QVteImpl;
    QPointer<QKxHistory> m_history;
    QPointer<QKxHistory> m_historyFile;
    QVector<TermLine> m_image;
    QBitArray m_tabs;
    int m_rows;
    int m_columns;

    TermCursor m_cursorSave;
    TermCursor m_cursorNow;
    bool m_cursorVisible;
    bool m_screenReverse;

    QMap<QPoint, QPoint> m_selection;

    int m_icapture;
    QMap<int, QPoint> m_capture;
};

#endif // QSCREEN_H
