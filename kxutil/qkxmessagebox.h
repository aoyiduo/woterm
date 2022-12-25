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

#ifndef QKXMESSAGEBOX_H
#define QKXMESSAGEBOX_H

#include "qkxutil_share.h"

#include <QMessageBox>

class KXUTIL_EXPORT QKxMessageBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit QKxMessageBox(QWidget *parent = nullptr);
    explicit QKxMessageBox(Icon icon, const QString &title, const QString &text,
                StandardButtons buttons = NoButton, QWidget *parent = nullptr,
                Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    virtual ~QKxMessageBox();

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButton button0, QMessageBox::StandardButton button1 = NoButton);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &text,
                                            QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton question(QWidget *parent, const QString &title, const QString &text,
                                         QMessageBox::StandardButtons buttons = StandardButtons(Yes | No), QMessageBox::StandardButton defaultButton = NoButton);
    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title, const QString &text,
                                               QMessageBox::StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
private:
    static void prettyButtonText(QMessageBox *dlg, const QString &title, const QString &text, StandardButtons buttons, StandardButton defaultButton);
private:
    virtual void showEvent(QShowEvent *event);

private slots:
    void onAdjustSize();
};

#endif // QKXMESSAGEBOX_H
