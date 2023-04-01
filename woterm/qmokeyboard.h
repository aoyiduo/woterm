/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QMOKEYBOARD_H
#define QMOKEYBOARD_H

#include <QWidget>
#include <QIcon>
#include <QPointer>
#include <QList>

namespace Ui {
class QMoKeyboard;
}

class QMoRedPoint;
class QPushButton;

class QMoKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit QMoKeyboard(QWidget *parent = nullptr);
    ~QMoKeyboard();

    void setVNCPatch(bool on);
signals:
    void keyEvent(QKeyEvent *ev);
private slots:
    void onCharButtonClicked();
    void onSpaceButtonClicked();
    void onCapsButtonClicked();
    void onShiftButtonClicked();
    void onCtrlButtonClicked();
    void onAltButtonClicked();
    void onSymbolButtonClicked();
    void onBackButtonClicked();
    void onEnterButtonClicked();
    void onBackspaceButtonClicked();
    void onUpButtonClicked();
    void onDownButtonClicked();
    void onLeftButtonClicked();
    void onRightButtonClicked();
    void onTabButtonClicked();
    void onHistoryButtonClicked();
    void onDfButtonClicked();
    void onLsButtonClicked();
    void onCtrlCButtonClicked();
    void simulateKeyEvent(QChar c, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
private:
    bool isCapsLockON() const;
    void setCapsLockON(bool yes);

    bool isAltON() const;
    void setAltON(bool yes);

    bool isCtrlON() const;
    void setCtrlON(bool on);

    bool isShiftON() const;
    void setShiftON(bool on);

    QKeyEvent *buildKeyEvent(bool pressed, QChar c, Qt::Key key);
    QKeyEvent *buildKeyEvent(bool pressed, QChar c, Qt::Key key, Qt::KeyboardModifiers modifiers);

    Qt::Key toKey(QChar qc);

    void setToLetter();
    void setToDigitalSymbol(int idx);
private:
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void keyReleaseEvent(QKeyEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
private:
    Ui::QMoKeyboard *ui;

    QPointer<QMoRedPoint> m_capsOn, m_altOn, m_ctrlOn, m_shiftOn;
    QList<QPointer<QPushButton>> m_btns1, m_btns23;
    int m_pageIdx;
    QString m_letters, m_digitals, m_symbols,m_shiftSymbols;

    QPoint m_dragPosition;
    bool m_vncPatch;
};

#endif // QMOKEYBOARD_H
