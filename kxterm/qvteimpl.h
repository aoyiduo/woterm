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

#ifndef QVTEIMPL_H
#define QVTEIMPL_H

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QFont>
#include <QMap>

#include "qvte.h"

class QTextCodec;
class QKxScreen;
class QKxHistory;

class QVteImpl : public QObject, public QVte
{
    Q_OBJECT
private:
    enum UpdateFlag {
        UF_Normal = 0x1,
        UF_FullScreen = 0x2
    };
    Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)
public:
    explicit QVteImpl(QObject *parent = nullptr);
    virtual ~QVteImpl();
    void setFont(const QFont& ft);

    QString textCodec() const;
    void setTextCodec(const QString& code);

    void process(const QByteArray& data);
    QKxScreen *screen();
    bool appMode();

    void clearAll();

    int historySize();
    void setHistorySize(int s);
    void setHistoryFile(const QString& file);
    void stopHistoryFile();
    void cleanHistory();
    void resetState();
signals:
    void sendData(const QByteArray& data);
    void screenChanged();
    void contentChanged(bool full);
    void titleChanged(const QString& title);
private:
    virtual void screenResize(int rows, int cols);
    virtual void resetColor(int c);
    virtual void defineColor(int c, const wchar_t *s);
    virtual void defineCharset(int n, TermCharset cs);
    virtual void selectCharset(int n);
    virtual void showCursor(const TermCursor& tc);
    virtual void hideCursor(const TermCursor& tc);
    virtual void clearTabs();
    virtual void setTab(int x);
    virtual void resetTab(int x);
    virtual int getNextTab(int x, int n);
    virtual int getCharWidth(wchar_t c, const TermCursor& tc);
    virtual void putReversedQuestionMark(const TermCursor& tc);
    virtual void putChar(wchar_t c, int char_width, const TermCursor& tc);
    virtual bool requestWidth(int width);
    virtual void clearRegion(int x1, int y1, int x2, int y2, const TermCursor& tc);
    virtual void insertChars(int x, int n, const TermCursor& tc);
    virtual void deleteChars(int x, int n, const TermCursor& tc);
    virtual void scrollUp(int y1, int y2, int n);
    virtual void scrollDown(int y1, int y2, int n);
    virtual void bell();
    virtual void setLineTextSize(int y, TermTextSize t);
    virtual void ttyWrite(const char *s);
    virtual void setWindowTitle(const wchar_t *title);
    virtual void flagLockKeyboard(bool state);
    virtual void flagApplicationScreen(bool state);
    virtual void flagApplicationKeypad(bool state);
    virtual void flagCursorVisible(bool state);
    virtual void flagSoftScroll(bool state);
    virtual void flagScreenReversed(bool state);
    virtual void saveCursor(const TermCursor& cursor);
    virtual TermCursor restoreCursor() const;
    virtual void updateCursor(const TermCursor& cursor);
    virtual TermCursor cursorNow() const;
private:
    int graphicCount(wchar_t c);
    int specialCharactorCount(wchar_t c);
private:
    QMap<int, int> m_graphicCount;
    QHash<int, int> m_box;
    QTextCodec *m_codec;

    QPointer<QKxScreen> m_cmdScreen;
    QPointer<QKxScreen> m_appScreen;
    QPointer<QKxScreen> m_screen;
    int m_rows;
    int m_cols;

    QFont m_font;
    QString m_code;
    UpdateFlags m_flags;    
    QByteArray m_bufLeft;
};

#endif // QVTEIMPL_H
