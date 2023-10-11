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

#include "qkxshortcutinputdialog.h"
#include "ui_qkxshortcutinputdialog.h"

#include <QDebug>
#include <QMessageBox>

QKxShortcutInputDialog::QKxShortcutInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKxShortcutInputDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Shortcut input"));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->shortcut, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(onKeySequenceChanged()));
    QObject::connect(ui->shortcut, SIGNAL(editingFinished()), this, SLOT(onKeySequenceChanged()));
}

QKxShortcutInputDialog::~QKxShortcutInputDialog()
{
    delete ui;
}

void QKxShortcutInputDialog::init(const QKeySequence &key)
{
    ui->shortcut->setKeySequence(key);
}

QString QKxShortcutInputDialog::result() const
{
    QKeySequence key = ui->shortcut->keySequence();
    QString name = key.toString();
    QStringList all = name.split(",");
    return all.last();
}

void QKxShortcutInputDialog::onApplyButtonClicked()
{
    QKeySequence key = ui->shortcut->keySequence();
    QString txt = key.toString();
    if(txt.isEmpty()) {
        QMessageBox::information(this, tr("Parameter error"), tr("Please input a shortcut."));
        return;
    }
    done(QDialog::Accepted+1);
}

void QKxShortcutInputDialog::onKeySequenceChanged()
{
    QKeySequence seq = ui->shortcut->keySequence();
    QString txt = seq.toString();
    qDebug() << "onKeySequenceChanged" << txt;
    if(txt.isEmpty()) {
        return;
    }
    QStringList all = txt.split(",");
    if(all.length() > 1) {
        QString last = all.last();
        QKeySequence keyLast(last.simplified());
        ui->shortcut->clear();
        ui->shortcut->setKeySequence(keyLast);
        showTip(keyLast);
    }else{
        showTip(seq);
    }
}

void QKxShortcutInputDialog::showTip(const QKeySequence &seq)
{
    int ckey = seq[0];
    int vkey = ckey &~(Qt::MODIFIER_MASK);
    QKeySequence key(vkey);
    QString tip = key.toString() + " ";

    qDebug() << "showTip" << QString::number(ckey, 16);
    if(ckey & Qt::SHIFT) {
        tip += "+Shift";
    }else{
        tip += "-Shift";
    }
#if defined (Q_OS_MAC)
    if(ckey & Qt::CTRL) {
        tip += "+Control";
    }else{
        tip += "-Control";
    }
    if(ckey & Qt::ALT) {
        tip += "+Option";
    }else{
        tip += "-Option";
    }
    if(ckey & Qt::META) {
        tip += "+Command";
    }else{
        tip += "-Command";
    }
#else
    if(ckey & Qt::CTRL) {
        tip += "+Ctrl";
    }else{
        tip += "-Ctrl";
    }
    if(ckey & Qt::ALT) {
        tip += "+Alt";
    }else{
        tip += "-Alt";
    }
    if(ckey & Qt::META) {
        tip += "+Meta";
    }else{
        tip += "-Meta";
    }
#endif
    ui->tip->setText(tip);
}
