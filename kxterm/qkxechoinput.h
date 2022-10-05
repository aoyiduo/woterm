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

#ifndef QKXECHOINPUT_H
#define QKXECHOINPUT_H

#include "qkxterm_share.h"

#include <QObject>
#include <QPointer>
#include <QKeyEvent>

class QKxTermItem;
class QTERM_EXPORT QKxEchoInput : public QObject
{
    Q_OBJECT
public:
    explicit QKxEchoInput(QKxTermItem *term, QObject *parent = nullptr);
    void setEnterReturnCharacter(const QByteArray& ch);
    void tryToPaste(const QString& txt);
    void updateCursor(bool fromParser = true);
    void onKeyPressEvent(QKeyEvent *ev);
    void onKeyReleaseEvent(QKeyEvent *ev);
signals:
    void commandArrived(const QByteArray& line);
    void ctrlCArrived();
private slots:    
    void onTermSize(int lines, int columns);
private:
    void reset();
    void clearInputArea();
    Q_INVOKABLE void refreshInput(bool force = false);
private:
    QPointer<QKxTermItem> m_term;
    QByteArray m_enterReturnChar;
    QPoint m_ptInputStart;
    QSize m_size;
    bool m_enterNew;
    bool m_updateCursor;

    QString m_inputText;
    int m_idxCursorPosition, m_idxHistory;
    QStringList m_history;
};

#endif // QKXECHOINPUT_H
