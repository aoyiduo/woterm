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

#include "qmomessageboxassist.h"
#include "qkxmessagebox.h"

QMoMessageBoxAssist::QMoMessageBoxAssist(QWidget *parent)
    : QObject(parent)
    , m_dlgParent(parent)
{

}

int QMoMessageBoxAssist::critical(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::critical(m_dlgParent, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QMoMessageBoxAssist::information(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::information(m_dlgParent, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QMoMessageBoxAssist::question(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::question(m_dlgParent, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QMoMessageBoxAssist::warning(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::warning(m_dlgParent, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}
