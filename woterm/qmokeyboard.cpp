/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qmokeyboard.h"
#include "ui_qmokeyboard.h"

#include "qmoredpoint.h"

#include <QBoxLayout>
#include <QPixmap>
#include <QDebug>
#include <QKeyEvent>

#define RED_POINT_SIZE      (QSize(5,5))
#define RED_POINT_POINT     (QPoint(3,3))

QMoKeyboard::QMoKeyboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QMoKeyboard)
    , m_letters("QWERTYUIOPASDFGHJKLZXCVBNM")
    , m_digitals("1234567890")
    , m_symbols("~`!@#$%^&*()_-+={}[]|\\:;\"'<>,.?/")
    , m_shiftSymbols("~!@#$%^&*()_+{}|:\\\"<>?")
    , m_vncPatch(false)
    , m_dragEnabled(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);

    m_pageIdx = 0;
    m_btns1.append(ui->btnKeyQ);
    m_btns1.append(ui->btnKeyW);
    m_btns1.append(ui->btnKeyE);
    m_btns1.append(ui->btnKeyR);
    m_btns1.append(ui->btnKeyT);
    m_btns1.append(ui->btnKeyY);
    m_btns1.append(ui->btnKeyU);
    m_btns1.append(ui->btnKeyI);
    m_btns1.append(ui->btnKeyO);
    m_btns1.append(ui->btnKeyP);

    m_btns23.append(ui->btnKeyA);
    m_btns23.append(ui->btnKeyS);
    m_btns23.append(ui->btnKeyD);
    m_btns23.append(ui->btnKeyF);
    m_btns23.append(ui->btnKeyG);
    m_btns23.append(ui->btnKeyH);
    m_btns23.append(ui->btnKeyJ);
    m_btns23.append(ui->btnKeyK);
    m_btns23.append(ui->btnKeyL);

    m_btns23.append(ui->btnKeyZ);
    m_btns23.append(ui->btnKeyX);
    m_btns23.append(ui->btnKeyC);
    m_btns23.append(ui->btnKeyV);
    m_btns23.append(ui->btnKeyB);
    m_btns23.append(ui->btnKeyN);
    m_btns23.append(ui->btnKeyM);

    QList<QPointer<QPushButton>> tmps = m_btns1 + m_btns23;
    for(auto it = tmps.begin(); it != tmps.end(); it++) {
        QPushButton *btn = *it;
        QObject::connect(btn, SIGNAL(clicked()), this, SLOT(onCharButtonClicked()));
    }

    {
        int mh = ui->btnKeyEnter->height();
        QList<QPushButton*> tmps = findChildren<QPushButton*>();
        for(auto it = tmps.begin(); it != tmps.end(); it++) {
            QPushButton *btn = *it;
            btn->setMinimumHeight(mh);
        }
    }

    QObject::connect(ui->btnKeySpace, SIGNAL(clicked()), this, SLOT(onSpaceButtonClicked()));

    QObject::connect(ui->btnKeyCaps, SIGNAL(clicked()), this, SLOT(onCapsButtonClicked()));
    QObject::connect(ui->btnKeyShift, SIGNAL(clicked()), this, SLOT(onShiftButtonClicked()));
    QObject::connect(ui->btnKeyAlt, SIGNAL(clicked()), this, SLOT(onAltButtonClicked()));
    QObject::connect(ui->btnKeyCtrl, SIGNAL(clicked()), this, SLOT(onCtrlButtonClicked()));    
    QObject::connect(ui->btnKeySymAll, SIGNAL(clicked()), this, SLOT(onSymbolButtonClicked()));
    QObject::connect(ui->btnKeyEnter, SIGNAL(clicked()), this, SLOT(onEnterButtonClicked()));
    QObject::connect(ui->btnKeyDel, SIGNAL(clicked()), this, SLOT(onBackspaceButtonClicked()));
    QObject::connect(ui->btnKeyUp, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    QObject::connect(ui->btnKeyDown, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));
    QObject::connect(ui->btnKeyLeft, SIGNAL(clicked()), this, SLOT(onLeftButtonClicked()));
    QObject::connect(ui->btnKeyRight, SIGNAL(clicked()), this, SLOT(onRightButtonClicked()));
    QObject::connect(ui->btnKeyTab, SIGNAL(clicked()), this, SLOT(onTabButtonClicked()));
    QObject::connect(ui->btnKeyBack, SIGNAL(clicked()), this, SLOT(onBackButtonClicked()));
    QObject::connect(ui->btnKeyHistory, SIGNAL(clicked()), this, SLOT(onHistoryButtonClicked()));
    QObject::connect(ui->btnKeyDf, SIGNAL(clicked()), this, SLOT(onDfButtonClicked()));
    QObject::connect(ui->btnKeyLs, SIGNAL(clicked()), this, SLOT(onLsButtonClicked()));
    QObject::connect(ui->btnKeyCtrlC, SIGNAL(clicked()), this, SLOT(onCtrlCButtonClicked()));

    ui->btnKeyCmd->hide();

    adjustSize();

    setCapsLockON(true);
    onCapsButtonClicked();

    setShiftON(true);
    onShiftButtonClicked();

    setAltON(true);
    onAltButtonClicked();

    setCtrlON(true);
    onCtrlButtonClicked();
}

QMoKeyboard::~QMoKeyboard()
{
    delete ui;
}

void QMoKeyboard::setVNCPatch(bool on)
{
    m_vncPatch = on;
}

bool QMoKeyboard::dragEnabled()
{
    return m_dragEnabled;
}

void QMoKeyboard::setDragEnabled(bool on)
{
    m_dragEnabled = on;
}

void QMoKeyboard::onCharButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    QString str = btn->text();
    if(str == "&&") {
        str = "&";
    }
    if(str.length() == 1) {
        //qDebug() << "onCharButtonClicked" << str;
        QChar c = str.at(0);
        Qt::Key key = toKey(c);
        if(m_vncPatch && m_shiftSymbols.indexOf(c) >= 0) {
            // fix for vnc.
            if(!isShiftON()) {
                emit keyEvent(buildKeyEvent(true, 0, Qt::Key_Shift));
            }
            emit keyEvent(buildKeyEvent(true, c, key));
            emit keyEvent(buildKeyEvent(false, c, key));
            if(!isShiftON()) {
                emit keyEvent(buildKeyEvent(false, 0, Qt::Key_Shift));
            }
        }else{
            emit keyEvent(buildKeyEvent(true, c, key));
            emit keyEvent(buildKeyEvent(false, c, key));
        }
    }
}

void QMoKeyboard::onSpaceButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, QChar::Space , Qt::Key_Space));
    emit keyEvent(buildKeyEvent(false, QChar::Space, Qt::Key_Space));
}

void QMoKeyboard::onCapsButtonClicked()
{
    bool on = !isCapsLockON();
    setCapsLockON(on);

    setToLetter();
}

void QMoKeyboard::onShiftButtonClicked()
{
    bool on = !isShiftON();
    setShiftON(on);
}

void QMoKeyboard::onCtrlButtonClicked()
{
    bool on = !isCtrlON();
    setCtrlON(on);
}

void QMoKeyboard::onAltButtonClicked()
{
    bool on = !isAltON();
    setAltON(on);
}

void QMoKeyboard::onSymbolButtonClicked()
{
    int totalPage = (m_symbols.length() + m_btns23.length() - 1) / m_btns23.length();
    if(m_pageIdx >= totalPage) {
        m_pageIdx = 0;
    }
    setToDigitalSymbol(m_pageIdx);
    m_pageIdx++;
}

void QMoKeyboard::onEnterButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0x0D , Qt::Key_Enter));
    emit keyEvent(buildKeyEvent(false, 0x0D, Qt::Key_Enter));
}

void QMoKeyboard::onBackspaceButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0x08, Qt::Key_Backspace));
    emit keyEvent(buildKeyEvent(false, 0x08, Qt::Key_Backspace));
}

void QMoKeyboard::onUpButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0, Qt::Key_Up));
    emit keyEvent(buildKeyEvent(false, 0, Qt::Key_Up));
}

void QMoKeyboard::onDownButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0, Qt::Key_Down));
    emit keyEvent(buildKeyEvent(false, 0, Qt::Key_Down));
}

void QMoKeyboard::onLeftButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0, Qt::Key_Left));
    emit keyEvent(buildKeyEvent(false, 0, Qt::Key_Left));
}

void QMoKeyboard::onRightButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, 0, Qt::Key_Right));
    emit keyEvent(buildKeyEvent(false, 0, Qt::Key_Right));
}

void QMoKeyboard::onTabButtonClicked()
{
    emit keyEvent(buildKeyEvent(true, '\t', Qt::Key_Tab));
    emit keyEvent(buildKeyEvent(false, '\t', Qt::Key_Tab));
}

void QMoKeyboard::onHistoryButtonClicked()
{
    QString cmd = "history";
    for(int i = 0; i < cmd.length(); i++) {
        QChar c = cmd.at(i);
        simulateKeyEvent(c);
    }
}

void QMoKeyboard::onDfButtonClicked()
{
    QString cmd = "df -h";
    for(int i = 0; i < cmd.length(); i++) {
        QChar c = cmd.at(i);
        simulateKeyEvent(c);
    }
}

void QMoKeyboard::onLsButtonClicked()
{
    QString cmd = "ls -l";
    for(int i = 0; i < cmd.length(); i++) {
        QChar c = cmd.at(i);
        simulateKeyEvent(c);
    }
}

void QMoKeyboard::onCtrlCButtonClicked()
{
    QString cmd = "C";
    for(int i = 0; i < cmd.length(); i++) {
        QChar c = cmd.at(i);
        simulateKeyEvent(c, Qt::ControlModifier);
    }
}

void QMoKeyboard::simulateKeyEvent(QChar c, Qt::KeyboardModifiers modifiers)
{
    if(c == QChar::Space) {
        emit keyEvent(buildKeyEvent(true, c, Qt::Key_Space, modifiers));
        emit keyEvent(buildKeyEvent(false, c, Qt::Key_Space, modifiers));
    }else if(c.isDigit()) {
        int offset = c.unicode() - QChar('0').unicode();
        Qt::Key key = Qt::Key(Qt::Key_0 + offset);
        emit keyEvent(buildKeyEvent(true, c, key, modifiers));
        emit keyEvent(buildKeyEvent(false, c, key, modifiers));
    }else if(c.isLetter()) {
        QChar t = c.toUpper();
        int offset = t.unicode() - QChar('A').unicode();
        Qt::Key key = Qt::Key(Qt::Key_A + offset);
        emit keyEvent(buildKeyEvent(true, c, key, modifiers));
        emit keyEvent(buildKeyEvent(false, c, key, modifiers));
    }else{
        Qt::Key key = Qt::Key(c.unicode());
        emit keyEvent(buildKeyEvent(true, c, key, modifiers));
        emit keyEvent(buildKeyEvent(false, c, key, modifiers));
    }
}

void QMoKeyboard::onBackButtonClicked()
{
    setToLetter();
}

bool QMoKeyboard::isCapsLockON() const
{
    QVariant v = ui->btnKeyCaps->property("stateON");
    return v.toBool();
}

void QMoKeyboard::setCapsLockON(bool yes)
{
    ui->btnKeyCaps->setProperty("stateON", yes);
    if(m_capsOn == nullptr) {
        m_capsOn = new QMoRedPoint(RED_POINT_SIZE, ui->btnKeyCaps);
        m_capsOn->move(RED_POINT_POINT);
    }
    m_capsOn->setVisible(yes);
}

bool QMoKeyboard::isAltON() const
{
    QVariant v = ui->btnKeyAlt->property("stateON");
    return v.toBool();
}

void QMoKeyboard::setAltON(bool yes)
{
    ui->btnKeyAlt->setProperty("stateON", yes);
    if(m_altOn == nullptr) {
        m_altOn = new QMoRedPoint(RED_POINT_SIZE, ui->btnKeyAlt);
        m_altOn->move(RED_POINT_POINT);
    }
    m_altOn->setVisible(yes);
    emit keyEvent(buildKeyEvent(yes, 0, Qt::Key_Alt));
}

bool QMoKeyboard::isCtrlON() const
{
    QVariant v = ui->btnKeyCtrl->property("stateON");
    return v.toBool();
}

void QMoKeyboard::setCtrlON(bool yes)
{
    ui->btnKeyCtrl->setProperty("stateON", yes);
    if(m_ctrlOn == nullptr) {
        m_ctrlOn = new QMoRedPoint(RED_POINT_SIZE, ui->btnKeyCtrl);
        m_ctrlOn->move(RED_POINT_POINT);
    }
    m_ctrlOn->setVisible(yes);
    emit keyEvent(buildKeyEvent(yes, 0, Qt::Key_Control));
}

bool QMoKeyboard::isShiftON() const
{
    QVariant v = ui->btnKeyShift->property("stateON");
    return v.toBool();
}

void QMoKeyboard::setShiftON(bool yes)
{
    ui->btnKeyShift->setProperty("stateON", yes);
    if(m_shiftOn == nullptr) {
        m_shiftOn = new QMoRedPoint(RED_POINT_SIZE, ui->btnKeyShift);
        m_shiftOn->move(RED_POINT_POINT);
    }
    m_shiftOn->setVisible(yes);
    emit keyEvent(buildKeyEvent(yes, 0, Qt::Key_Shift));
}

QKeyEvent *QMoKeyboard::buildKeyEvent(bool pressed, QChar c, Qt::Key key)
{
    Qt::KeyboardModifiers modifiers = 0;
    if(isShiftON()) {
        modifiers |= Qt::ShiftModifier;
    }
    if(isCtrlON()) {
        modifiers |= Qt::ControlModifier;
    }
    if(isAltON()) {
        modifiers |= Qt::AltModifier;
    }
    if(modifiers == Qt::ShiftModifier) {
        c = c.isLower() ? c.toUpper() : c.toLower();
    }else if((modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier)) {
        c = c.toUpper();
    }
    return buildKeyEvent(pressed, c, key, modifiers);
}

QKeyEvent *QMoKeyboard::buildKeyEvent(bool pressed, QChar c, Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    QKeyEvent *ke = new QKeyEvent(pressed ? QEvent::KeyPress : QEvent::KeyRelease, key, modifiers, c);
    return ke;
}

Qt::Key QMoKeyboard::toKey(QChar qc)
{
    char c = qc.toLatin1();
    if(c >= 'a' && c <= 'z') {
        return Qt::Key(Qt::Key_A + c - 'a');
    }
    if(c >= 'A' && c <= 'Z') {
        return Qt::Key(Qt::Key_A + c - 'A');
    }
    if(c >= '0' && c <= '9') {
        return Qt::Key(Qt::Key_0 + c - '0');
    }
    // symbol's unicode value are equal to Qt::Key;
    return Qt::Key(qc.unicode());
}

void QMoKeyboard::setToLetter()
{
    m_pageIdx = 0;
    ui->btnKeyBack->setVisible(false);
    ui->btnKeyCaps->setVisible(true);
    m_letters = isCapsLockON() ? m_letters.toUpper() : m_letters.toLower();
    QList<QPointer<QPushButton>> tmp = m_btns1 + m_btns23;
    for(int i = 0; i < tmp.length(); i++) {
        tmp[i]->setText(m_letters.at(i));
    }
}

void QMoKeyboard::setToDigitalSymbol(int page)
{
    ui->btnKeyBack->setVisible(true);
    ui->btnKeyCaps->setVisible(false);
    for(int i = 0; i < m_btns1.length(); i++) {
        m_btns1[i]->setText(m_digitals.at(i));
    }
    for(int i = 0; i < m_btns23.length(); i++) {
        int idx = page * m_btns23.length() + i;
        if(idx >= m_symbols.length()) {
            m_btns23[i]->setText("");
        }else{
            QString tmp = m_symbols.at(idx);
            if(tmp == "&") {
                tmp = "&&";
            }
            m_btns23[i]->setText(tmp);
        }
    }
}


void QMoKeyboard::keyPressEvent(QKeyEvent *ev)
{
    QString txt = ev->text();
    if(!txt.isEmpty()) {
        QChar c = txt.at(0);
        qDebug() << "keyPressEvent" << ev->key() << c.unicode() << ev->text();
    }else{
        qDebug() << "keyPressEvent" << ev->key();
    }
}

void QMoKeyboard::keyReleaseEvent(QKeyEvent *ev)
{

}

void QMoKeyboard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragEnabled && event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void QMoKeyboard::mousePressEvent(QMouseEvent *event)
{
    if (m_dragEnabled && event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
