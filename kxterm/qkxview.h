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

#ifndef QVIEW_H
#define QVIEW_H

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QMap>


#include "qvtedef.h"

class QKxScreen;

class QKxView : public QObject
{
    Q_OBJECT
public:
    explicit QKxView(QObject *parent = nullptr);
    ~QKxView();
    void setScreen(QKxScreen *p);
    QList<TermLine> viewport(int y, int rows, int cols, bool *reverse, SelectionMode m);    
    int lineCount() const;
    int historyLineCount() const;
    TermCursor cursor() const;
    QPoint cursorToScreenPosition() const;
    QPoint cursorToViewPosition() const;
    QPoint viewToScreenPosition(const QPoint& pt) const;

    // coordinate is according to global view not screen.
    void clearSelection();
    QMap<QPoint,QPoint> selection();
    void setSelection(QPoint start, QPoint end);
    void setSelection(const QMap<QPoint, QPoint>& sels);
    void selectWord(QPoint pt);
    void selectLine(int y);
    void setSelectionMode();    
    TermLine lineAt(int y) const;

    QString plainText(const QPoint& start, const QPoint& end, SelectionMode m=SM_LineText) const;
    QString selectedText();
    QList<TermLine> selectedLines();

    // search
    bool find(const QString& key, bool match, bool regular);
    bool findPrev(bool match, bool regular);
    bool findNext(bool match, bool regular);
    void findAll(bool match, bool regular);

    // capture.
    void clearCapture();
    int setCapture(const QPoint& pt);
    bool releaseCapture(int id, QPoint* pt);
signals:
    void selectChanged();

private:
    QList<TermLine> copyImage(int y, int rows, int cols);
    void markSelection(QList<TermLine>& img, int y, int rows, int cols, SelectionMode m);
    QChar typeCheck(QChar c);
    bool lineText(int y, QString& out, QList<int> &pos);
    void cleanFind();
private:
    QPointer<QKxScreen> m_screen;
    SelectionMode m_mode;
    QString m_selectedText;
    QString m_findText;
    int m_findKey;
    int m_findLength;
};

#endif // QVIEW_H
