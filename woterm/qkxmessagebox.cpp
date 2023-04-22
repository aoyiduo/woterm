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

#include "qkxmessagebox.h"

#include <QAbstractButton>
#include <QTimer>

QKxMessageBox::QKxMessageBox(QWidget *parent)
    : QMessageBox(parent)
{

}

QKxMessageBox::QKxMessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons, QWidget *parent, Qt::WindowFlags flags)
    : QMessageBox(icon, title, text, buttons, parent, flags)
{
    prettyButtonText(this, title, text, buttons, NoButton);
}

QKxMessageBox::~QKxMessageBox()
{

}

QMessageBox::StandardButton QKxMessageBox::critical(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
    QKxMessageBox dlg(parent);
    prettyButtonText(&dlg, title, text, buttons, defaultButton);
    dlg.setIcon(QMessageBox::Critical);
    return (QMessageBox::StandardButton)dlg.exec();
}

QMessageBox::StandardButton QKxMessageBox::information(QWidget *parent, const QString &title, const QString &text, StandardButton button0, StandardButton button1)
{
    QKxMessageBox dlg(parent);
    prettyButtonText(&dlg, title, text, button0|button1, button0);
    dlg.setIcon(QMessageBox::Critical);
    return (QMessageBox::StandardButton)dlg.exec();
}

QMessageBox::StandardButton QKxMessageBox::information(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
    QKxMessageBox dlg(parent);
    prettyButtonText(&dlg, title, text, buttons, defaultButton);
    dlg.setIcon(QMessageBox::Information);
    return (QMessageBox::StandardButton)dlg.exec();
}

QMessageBox::StandardButton QKxMessageBox::question(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
    QKxMessageBox dlg(parent);
    prettyButtonText(&dlg, title, text, buttons, defaultButton);
    dlg.setIcon(QMessageBox::Question);
    return (QMessageBox::StandardButton)dlg.exec();
}

QMessageBox::StandardButton QKxMessageBox::warning(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
    QKxMessageBox dlg(parent);
    prettyButtonText(&dlg, title, text, buttons, defaultButton);
    dlg.setIcon(QMessageBox::Warning);
    return (QMessageBox::StandardButton)dlg.exec();
}

void QKxMessageBox::prettyButtonText(QMessageBox *dlg, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton)
{
    dlg->setWindowTitle(title);
    dlg->setText(text);
    dlg->setStandardButtons(buttons);
    dlg->setDefaultButton(defaultButton);
    if(buttons.testFlag(QMessageBox::Ok)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Ok);
        btn->setText(tr("Ok"));
    }
    if(buttons.testFlag(QMessageBox::Save)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Save);
        btn->setText(tr("Save"));
    }
    if(buttons.testFlag(QMessageBox::SaveAll)) {
        QAbstractButton *btn = dlg->button(QMessageBox::SaveAll);
        btn->setText(tr("Save all"));
    }
    if(buttons.testFlag(QMessageBox::Open)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Open);
        btn->setText(tr("Open"));
    }
    if(buttons.testFlag(QMessageBox::Yes)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Yes);
        btn->setText(tr("Yes"));
    }
    if(buttons.testFlag(QMessageBox::YesToAll)) {
        QAbstractButton *btn = dlg->button(QMessageBox::YesToAll);
        btn->setText(tr("Yes to all"));
    }
    if(buttons.testFlag(QMessageBox::No)) {
        QAbstractButton *btn = dlg->button(QMessageBox::No);
        btn->setText(tr("No"));
    }
    if(buttons.testFlag(QMessageBox::NoToAll)) {
        QAbstractButton *btn = dlg->button(QMessageBox::NoToAll);
        btn->setText(tr("No to all"));
    }
    if(buttons.testFlag(QMessageBox::Abort)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Abort);
        btn->setText(tr("Abort"));
    }
    if(buttons.testFlag(QMessageBox::Retry)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Retry);
        btn->setText(tr("Retry"));
    }
    if(buttons.testFlag(QMessageBox::Ignore)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Ignore);
        btn->setText(tr("Ignore"));
    }
    if(buttons.testFlag(QMessageBox::Close)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Close);
        btn->setText(tr("Close"));
    }
    if(buttons.testFlag(QMessageBox::Cancel)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Cancel);
        btn->setText(tr("Cancel"));
    }
    if(buttons.testFlag(QMessageBox::Discard)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Discard);
        btn->setText(tr("Discard"));
    }
    if(buttons.testFlag(QMessageBox::Help)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Help);
        btn->setText(tr("Help"));
    }
    if(buttons.testFlag(QMessageBox::Apply)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Apply);
        btn->setText(tr("Apply"));
    }
    if(buttons.testFlag(QMessageBox::Reset)) {
        QAbstractButton *btn = dlg->button(QMessageBox::Reset);
        btn->setText(tr("Reset"));
    }
    if(buttons.testFlag(QMessageBox::RestoreDefaults)) {
        QAbstractButton *btn = dlg->button(QMessageBox::RestoreDefaults);
        btn->setText(tr("Restore defaults"));
    }
}

void QKxMessageBox::showEvent(QShowEvent *event)
{
    QMessageBox::showEvent(event);
    // has nothing effect.
    //QTimer::singleShot(1, this, SLOT(onAdjustSize()));
}

void QKxMessageBox::onAdjustSize()
{
    QSize sz = minimumSize();
    setMinimumSize(sz.width() + 300, sz.height()+50);
    adjustSize();
}
