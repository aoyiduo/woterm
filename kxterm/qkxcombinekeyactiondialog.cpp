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

#include "qkxcombinekeyactiondialog.h"
#include "ui_qkxcombinekeyactiondialog.h"

#include "qkxshortcutinputdialog.h"

#include <QButtonGroup>
#include <QTimer>
#include <QStringListModel>
#include <QDateTime>
#include <QDebug>
#include <QKeyEvent>

#define SIMULATE_MAC_KEY    (false)

QKxCombineKeyActionDialog::QKxCombineKeyActionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKxCombineKeyActionDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Combine key action input"));

    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    QStringListModel *model = new QStringListModel(this);
    auto all = QKxKeyTranslator::operationDescriptions();
    model->setStringList(all.values());
    ui->op->setModel(model);
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radAnsiN);
        group->addButton(ui->radAnsiO);
        group->addButton(ui->radAnsiP);
        ui->radAnsiO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radNewLineN);
        group->addButton(ui->radNewLineO);
        group->addButton(ui->radNewLineP);
        ui->radNewLineO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radAppCuKeysN);
        group->addButton(ui->radAppCuKeysO);
        group->addButton(ui->radAppCuKeysP);
        ui->radAppCuKeysO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radAppScreenN);
        group->addButton(ui->radAppScreenO);
        group->addButton(ui->radAppScreenP);
        ui->radAppScreenO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radAppKeyPadN);
        group->addButton(ui->radAppKeyPadO);
        group->addButton(ui->radAppKeyPadP);
        ui->radAppKeyPadO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ui->radAltN->setText("-Option");
        ui->radAltO->setText("None");
        ui->radAltP->setText("+Option");
#endif
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radAltN);
        group->addButton(ui->radAltO);
        group->addButton(ui->radAltP);
        ui->radAltO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ui->radCtrlN->setText("-Control");
        ui->radCtrlO->setText("None");
        ui->radCtrlP->setText("+Control");
#endif
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radCtrlN);
        group->addButton(ui->radCtrlO);
        group->addButton(ui->radCtrlP);
        ui->radCtrlO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radShiftN);
        group->addButton(ui->radShiftO);
        group->addButton(ui->radShiftP);
        ui->radShiftO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ui->radMetaN->setText("-Command");
        ui->radMetaO->setText("None");
        ui->radMetaP->setText("+Command");
#endif
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radMetaN);
        group->addButton(ui->radMetaO);
        group->addButton(ui->radMetaP);
        ui->radMetaO->setChecked(true);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onCombineKeyChanged()));
    }
    {
        QButtonGroup *group = new QButtonGroup(this);
        group->addButton(ui->radString);
        group->addButton(ui->radOperation);
        QObject::connect(group, SIGNAL(buttonToggled(int,bool)), this, SLOT(onActionButtonClicked()));
        ui->radOperation->setChecked(true);
    }
    QObject::connect(ui->btnQuickInput, SIGNAL(clicked()), this, SLOT(onQuickInput()));
    ui->key->setReadOnly(false);
    ui->key->installEventFilter(this);
    QObject::connect(ui->key, SIGNAL(textChanged(QString)), this, SLOT(onCombineKeyChanged()));
    adjustSizeLater();
}

QKxCombineKeyActionDialog::~QKxCombineKeyActionDialog()
{
    delete ui;
}

void QKxCombineKeyActionDialog::init(const QKxKeyTranslator::KeyInfo &ki)
{
    if(ki.modes & QKxKeyTranslator::VM_ANSI_YES) {
        ui->radAnsiP->setChecked(true);
    }else if(ki.modes& QKxKeyTranslator::VM_ANSI_NO) {
        ui->radAnsiN->setChecked(true);
    }else{
        ui->radAnsiO->setChecked(true);
    }

    if(ki.modes & QKxKeyTranslator::VM_NEWLINE_YES) {
        ui->radNewLineP->setChecked(true);
    }else if(ki.modes& QKxKeyTranslator::VM_NEWLINE_NO) {
        ui->radNewLineN->setChecked(true);
    }else{
        ui->radNewLineO->setChecked(true);
    }

    if(ki.modes & QKxKeyTranslator::VM_APPCUKEY_YES) {
        ui->radAppCuKeysP->setChecked(true);
    }else if(ki.modes& QKxKeyTranslator::VM_APPCUKEY_NO) {
        ui->radAppCuKeysN->setChecked(true);
    }else{
        ui->radAppCuKeysO->setChecked(true);
    }

    if(ki.modes & QKxKeyTranslator::VM_APPSCREEN_YES) {
        ui->radAppScreenP->setChecked(true);
    }else if(ki.modes& QKxKeyTranslator::VM_APPSCREEN_NO) {
        ui->radAppScreenN->setChecked(true);
    }else{
        ui->radAppScreenO->setChecked(true);
    }

    if(ki.modes & QKxKeyTranslator::VM_APPKEYPAD_YES) {
        ui->radAppKeyPadP->setChecked(true);
    }else if(ki.modes& QKxKeyTranslator::VM_APPKEYPAD_NO) {
        ui->radAppKeyPadN->setChecked(true);
    }else{
        ui->radAppKeyPadO->setChecked(true);
    }

    if(ki.modifies & QKxKeyTranslator::MK_SHIFT_YES) {
        ui->radShiftP->setChecked(true);
    }else if(ki.modifies & QKxKeyTranslator::MK_SHIFT_NO) {
        ui->radShiftN->setChecked(true);
    }else{
        ui->radShiftO->setChecked(true);
    }

    if(ki.modifies & QKxKeyTranslator::MK_CTRL_YES) {
        ui->radCtrlP->setChecked(true);
    }else if(ki.modifies & QKxKeyTranslator::MK_CTRL_NO) {
        ui->radCtrlN->setChecked(true);
    }else{
        ui->radCtrlO->setChecked(true);
    }

    if(ki.modifies & QKxKeyTranslator::MK_ALT_YES) {
        ui->radAltP->setChecked(true);
    }else if(ki.modifies & QKxKeyTranslator::MK_ALT_NO) {
        ui->radAltN->setChecked(true);
    }else{
        ui->radAltO->setChecked(true);
    }

    if(ki.modifies & QKxKeyTranslator::MK_META_YES) {
        ui->radMetaP->setChecked(true);
    }else if(ki.modifies & QKxKeyTranslator::MK_META_NO) {
        ui->radMetaN->setChecked(true);
    }else{
        ui->radMetaO->setChecked(true);
    }

    QKeySequence seq(ki.key);
    ui->key->setText(seq.toString());

    ui->radString->setChecked(!ki.out.isEmpty());
    ui->radOperation->setChecked(ki.out.isEmpty());
    ui->stringArea->setVisible(ui->radString->isChecked());
    ui->opArea->setVisible(ui->radOperation->isChecked());

    if(ki.out.isEmpty()) {
        ui->op->setCurrentIndex(ki.op - 1);
    }else{
        ui->ansiText->setText(ki.action);
    }


    adjustSizeLater();
}

QKxKeyTranslator::KeyInfo QKxCombineKeyActionDialog::result()
{
    QKxKeyTranslator::KeyInfo ki;
    if(ui->radString->isChecked()) {
        ki.action = ui->ansiText->text();
        ki.out = QKxKeyTranslator::stringToAnsiSequence(ki.action);
    }else if(ui->radOperation->isChecked()) {
        int idx = ui->op->currentIndex();
        ki.op = QKxKeyTranslator::EOperation(idx+1);
        ki.action = QKxKeyTranslator::operationToName(ki.op);
    }

    if(ui->radAnsiP->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_ANSI_YES;
    }else if(ui->radAnsiN->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_ANSI_NO;
    }
    if(ui->radNewLineP->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_NEWLINE_YES;
    }else if(ui->radNewLineN->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_NEWLINE_NO;
    }
    if(ui->radAppCuKeysP->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPCUKEY_YES;
    }else if(ui->radAppCuKeysN->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPCUKEY_NO;
    }
    if(ui->radAppScreenP->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPSCREEN_YES;
    }else if(ui->radAppScreenN->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPSCREEN_NO;
    }
    if(ui->radAppKeyPadP->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPKEYPAD_YES;
    }else if(ui->radAppKeyPadN->isChecked()) {
        ki.modes |= QKxKeyTranslator::VM_APPKEYPAD_NO;
    }

    if(ui->radShiftP->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_SHIFT_YES;
    }else if(ui->radShiftN->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_SHIFT_NO;
    }
    if(ui->radCtrlP->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_CTRL_YES;
    }else if(ui->radCtrlN->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_CTRL_NO;
    }
    if(ui->radAltP->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_ALT_YES;
    }else if(ui->radAltN->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_ALT_NO;
    }
    if(ui->radMetaP->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_META_YES;
    }else if(ui->radMetaN->isChecked()) {
        ki.modifies |= QKxKeyTranslator::MK_META_NO;
    }
    QKeySequence seq(ui->key->text());
    ki.key = seq[0];
    ki.condition = combineKey();
    return ki;
}

void QKxCombineKeyActionDialog::onActionButtonClicked()
{
    //qDebug() << "onActionButtonClicked" << QDateTime::currentMSecsSinceEpoch();
    setUpdatesEnabled(false);
    ui->opArea->setVisible(ui->radOperation->isChecked());
    ui->stringArea->setVisible(ui->radString->isChecked());
    setUpdatesEnabled(true);
    adjustSizeLater();
}

void QKxCombineKeyActionDialog::onApplyButtonClicked()
{
    QString key = ui->key->text();
    if(key.isEmpty()) {
        emit messageArrived(tr("Parameter error"), tr("The key parameter should not be empty."));
        return;
    }
    if(ui->radString->isChecked()) {
        QString out = ui->ansiText->text();
        if(out.isEmpty()) {
            emit messageArrived(tr("Parameter error"), tr("The string parameter should not be empty."));
            return;
        }
    }
    done(QDialog::Accepted+1);
}

void QKxCombineKeyActionDialog::onCombineKeyChanged()
{
    ui->ckey->setText(combineKey());
}

void QKxCombineKeyActionDialog::onQuickInput()
{
    QKxShortcutInputDialog dlg(this);
    QString key = ui->key->text();
    if(!key.isEmpty()) {
        QKeySequence seq(key);
        int ckey = seq[0];
        if(ui->radShiftP->isChecked()) {
            ckey += Qt::SHIFT;
        }
        if(ui->radCtrlP->isChecked()) {
            ckey += Qt::CTRL;
        }
        if(ui->radAltP->isChecked()) {
            ckey += Qt::ALT;
        }
        if(ui->radMetaP->isChecked()) {
            ckey += Qt::META;
        }

        dlg.init(QKeySequence(ckey));
    }
    if(dlg.exec() == QDialog::Accepted+1) {
        QString key = dlg.result();
        QKeySequence seq(key);
        setShortcut(seq);
    }
}

void QKxCombineKeyActionDialog::adjustSizeLater()
{
    QPointer<QKxCombineKeyActionDialog> that(this);
    QTimer::singleShot(0, this, [=](){
        if(that == nullptr) {
            return ;
        }
        adjustSize();
    });
}

QString QKxCombineKeyActionDialog::combineKey()
{
    QString ckey = ui->key->text();
    ckey+= " ";
    if(ui->radShiftP->isChecked()) {
        ckey += "+Shift";
    }else if(ui->radShiftN->isChecked()) {
        ckey += "-Shift";
    }
    if(ui->radCtrlP->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "+Control";
#else
        ckey += "+Ctrl";
#endif
    }else if(ui->radCtrlN->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "-Control";
#else
        ckey += "-Ctrl";
#endif
    }
    if(ui->radAltP->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "+Option";
#else
        ckey += "+Alt";
#endif
    }else if(ui->radAltN->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "-Option";
#else
        ckey += "-Alt";
#endif
    }
    if(ui->radMetaP->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "+Command";
#else
        ckey += "+Meta";
#endif
    }else if(ui->radMetaN->isChecked()) {
#if defined (Q_OS_MAC) || SIMULATE_MAC_KEY
        ckey += "-Command";
#else
        ckey += "-Meta";
#endif
    }

    if(ui->radAnsiP->isChecked()) {
        ckey += "+Ansi";
    }else if(ui->radAnsiN->isChecked()) {
        ckey += "-Ansi";
    }
    if(ui->radNewLineP->isChecked()) {
        ckey += "+NewLine";
    }else if(ui->radNewLineN->isChecked()) {
        ckey += "-NewLine";
    }
    if(ui->radAppCuKeysP->isChecked()) {
        ckey += "+AppCuKeys";
    }else if(ui->radAppCuKeysN->isChecked()) {
        ckey += "-AppCuKeys";
    }
    if(ui->radAppScreenP->isChecked()) {
        ckey += "+AppScreen";
    }else if(ui->radAppScreenN->isChecked()) {
        ckey += "-AppScreen";
    }
    if(ui->radAppKeyPadP->isChecked()) {
        ckey += "+AppKeyPad";
    }else if(ui->radAppKeyPadN->isChecked()) {
        ckey += "-AppKeyPad";
    }
    return ckey;
}

void QKxCombineKeyActionDialog::setShortcut(const QKeySequence &seq)
{
    int ckey = seq[0];
    int vkey = ckey &~(Qt::MODIFIER_MASK);
    QKeySequence key(vkey);
    QString tmp = key.toString();
    ui->key->setText(tmp);
    if(ckey & Qt::SHIFT) {
        ui->radShiftP->setChecked(true);
    }else{
        ui->radShiftN->setChecked(true);
    }
    if(ckey & Qt::CTRL) {
        ui->radCtrlP->setChecked(true);
    }else{
        ui->radCtrlN->setChecked(true);
    }
    if(ckey & Qt::ALT) {
        ui->radAltP->setChecked(true);
    }else{
        ui->radAltN->setChecked(true);
    }
    if(ckey & Qt::META) {
        ui->radMetaP->setChecked(true);
    }else{
        ui->radMetaN->setChecked(true);
    }
}

bool QKxCombineKeyActionDialog::eventFilter(QObject *watched, QEvent *ev)
{
    QEvent::Type t = ev->type();
    if(watched == ui->key) {
        if(t == QEvent::KeyPress) {
            QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(ev);
            int key = ke->key();
            if(key == Qt::Key_Shift||
                    key == Qt::Key_Control||
                    key == Qt::Key_Alt||
                    key == Qt::Key_Meta) {
                return true;
            }
            QKeySequence seq(key);
            ui->key->setText(seq.toString());
            return true;
        }else if(t == QEvent::KeyRelease) {
            return true;
        }
    }
    return false;
}
