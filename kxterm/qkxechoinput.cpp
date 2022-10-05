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

#include "qkxechoinput.h"
#include "qkxtermitem.h"

#include <QDebug>

QKxEchoInput::QKxEchoInput(QKxTermItem *term, QObject *parent)
    : QObject(parent)
    , m_term(term)
    , m_enterNew(true)
    , m_updateCursor(false)
{
    m_enterReturnChar = "\r\n";
    m_idxCursorPosition = 0;
}

void QKxEchoInput::reset()
{
    m_size = m_term->termSize();
    m_ptInputStart = m_term->cursorToViewPosition();
}

void QKxEchoInput::setEnterReturnCharacter(const QByteArray &ch)
{
    m_enterReturnChar = ch;
}

void QKxEchoInput::tryToPaste(const QString &txt)
{
    if(m_enterNew) {
        m_enterNew = false;
        reset();
    }
    if(m_updateCursor) {
        m_updateCursor = false;
        m_ptInputStart = m_term->cursorToViewPosition();
    }
    if(m_inputText.length() == m_idxCursorPosition) {
        m_inputText.append(txt);
        m_idxCursorPosition = m_inputText.length();
    }else{
        m_inputText.insert(m_idxCursorPosition, txt);
        m_idxCursorPosition += txt.length();
    }
    refreshInput(true);
}

void QKxEchoInput::updateCursor(bool fromParser)
{
    m_updateCursor = true;
}

void QKxEchoInput::onKeyPressEvent(QKeyEvent *ev)
{
    QByteArray txt = ev->text().toUtf8();
    int key = ev->key();
    int code = ev->nativeScanCode();
    Qt::KeyboardModifiers modifiers = ev->modifiers();

    if((modifiers & Qt::ControlModifier) && key == Qt::Key_C) {
        emit ctrlCArrived();
        return;
    }

    if(m_enterNew) {
        //
        m_enterNew = false;
        reset();
    }
    if(m_updateCursor) {
        m_updateCursor = false;
        m_ptInputStart = m_term->cursorToViewPosition();
    }

    if(key >= Qt::Key_F1 && key <= Qt::Key_F29 && (modifiers & Qt::ControlModifier)) {

    }else if(key == Qt::Key_Enter || key == Qt::Key_Return) {
        clearInputArea();
        if(!m_history.contains(m_inputText)) {
            if(m_history.length() > 10) {
                m_history.pop_front();
            }
            m_history.append(m_inputText);
        }
        m_inputText.append(m_enterReturnChar);
        emit commandArrived(m_inputText.toUtf8());
        m_enterNew = true;
        m_inputText.clear();
        m_idxCursorPosition = 0;
    }else if(key == Qt::Key_Backspace) {
        m_idxCursorPosition--;
        if(m_idxCursorPosition < 0) {
            m_idxCursorPosition = 0;
        }
        m_inputText.remove(m_idxCursorPosition, 1);
    }else if(key == Qt::Key_Delete) {
        m_inputText.remove(m_idxCursorPosition, 1);
    }else if(key == Qt::Key_Home) {
        m_idxCursorPosition = 0;
    }else if(key == Qt::Key_End) {
        m_idxCursorPosition = m_inputText.length();
    }else if(key == Qt::Key_Left) {
        m_idxCursorPosition = qMax<int>(--m_idxCursorPosition, 0);
    }else if(key == Qt::Key_Right) {
        m_idxCursorPosition = qMin<int>(++m_idxCursorPosition, m_inputText.length());
    }else if(key == Qt::Key_Up) {
        if(!m_history.isEmpty()) {
            m_idxHistory--;
            if(m_idxHistory < 0) {
                m_idxHistory = m_history.length() - 1;
            }else if(m_idxHistory > m_history.length() - 1) {
                m_idxHistory = 0;
            }
            m_inputText = m_history.at(m_idxHistory);
            m_idxCursorPosition = m_inputText.length();
        }
    }else if(key == Qt::Key_Down) {
        if(!m_history.isEmpty()) {
            m_idxHistory--;
            if(m_idxHistory < 0) {
                m_idxHistory = m_history.length() - 1;
            }else if(m_idxHistory > m_history.length() - 1) {
                m_idxHistory = 0;
            }
            m_inputText = m_history.at(m_idxHistory);
            m_idxCursorPosition = m_inputText.length();
        }
    }else if(key == Qt::Key_Tab) {

    }else if(key == Qt::Key_PageUp || key == Qt::Key_PageDown) {
        // ignore.
    }else if(!txt.isEmpty()) {
        //
        if(m_inputText.length() == m_idxCursorPosition) {
            m_inputText.append(txt);
            m_idxCursorPosition = m_inputText.length();
        }else{
            m_inputText.insert(m_idxCursorPosition, txt);
            m_idxCursorPosition += txt.length();
        }
    }

    refreshInput(true);
}

void QKxEchoInput::onKeyReleaseEvent(QKeyEvent *ev)
{

}

void QKxEchoInput::onTermSize(int lines, int columns)
{
    m_size = QSize(columns, lines);
    QMetaObject::invokeMethod(this, "refreshInput", Qt::QueuedConnection);
}

void QKxEchoInput::clearInputArea()
{
    QPoint pt = m_term->viewToScreenPosition(m_ptInputStart);
    QString seq = QString("\x1b[%1;%2f\x1b[0J").arg(pt.y()+1).arg(pt.x()+1);
    m_term->echoInput(seq.toLatin1());
}

void QKxEchoInput::refreshInput(bool force)
{
    if(m_inputText.isEmpty() && !force) {
        return;
    }
    QPoint pt = m_term->viewToScreenPosition(m_ptInputStart);
    if(pt.y() < 0) {
        return;
    }
    int idx = pt.x() + m_idxCursorPosition;
    int ycnt = idx / m_size.width();
    int x = idx % m_size.width();
    int y = pt.y() + ycnt;
    if(y + 1 > m_size.height()) {
        m_term->echoInput("\x1b[1S");
        pt = m_term->viewToScreenPosition(m_ptInputStart);
    }
    QString seq = QString("\x1b[%1;%2f\x1b[0J").arg(pt.y()+1).arg(pt.x()+1);
    m_term->echoInput(seq.toUtf8());
    m_term->echoInput(m_inputText.toUtf8());
    QString cur = QString("\x1b[%1;%2f").arg(y+1).arg(x+1);
    m_term->echoInput(cur.toUtf8());
}
