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

#pragma once

#include <QObject>
#include <QPointer>
#include <QByteArray>

class QKxTermWidget;

class QWoLineNoise
{
protected:
    /* The linenoiseState structure represents the state during line editing.
     * We pass this state to functions implementing specific editing
     * functionalities. */
    typedef struct {
        QByteArray buf;          /* Edited line buffer. */
        int pos;         /* Current cursor position. */
        int oldpos;      /* Previous refresh cursor position. */
        int maxrows;     /* Maximum num of rows used so far (multiline mode) */
        QList<QByteArray> completes;
        int completeIndex;
    } LineNoiseState;

public:
    explicit QWoLineNoise(QKxTermWidget *term = nullptr);
    virtual ~QWoLineNoise();
    void parse(const QByteArray& buf);
    void setPrompt(const QByteArray& prompt);

protected:
    virtual void handleCommand(const QByteArray& line);
    virtual QList<QByteArray> handleComplete(const QByteArray& line);
    virtual QByteArray handleShowHints(QByteArray& line, int *pclr, int *pbold);
    virtual void parseSequenceText(const QByteArray& line);
protected:
    void resetInput();
private:
    void reset();
    void normalParse(const QByteArray& buf);

    void editInsert(char c);
    void editMoveEnd();
    void refreshLine();
    QByteArray refreshShowHints();
    int column();
    void editDelete();
    void editHistoryPrev();
    void editHistoryNext();
    void editMoveRight();
    void editMoveLeft();
    void editMoveHome();
    void editBackspace();
    void clearScreen();
    void editDeletePrevWord();
    void beep();
private:
    QPointer<QKxTermWidget> m_term;
    QByteArray m_prompt;    
    LineNoiseState m_state;
};
