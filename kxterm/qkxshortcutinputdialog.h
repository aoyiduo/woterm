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

#ifndef QKXSHORTCUTINPUTDIALOG_H
#define QKXSHORTCUTINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class QKxShortcutInputDialog;
}

class QKxShortcutInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QKxShortcutInputDialog(QWidget *parent = nullptr);
    ~QKxShortcutInputDialog();

    void init(const QKeySequence& key);
    QString result() const;
private slots:
    void onApplyButtonClicked();
    void onKeySequenceChanged();
private:
    void showTip(const QKeySequence& seq);
private:
    Ui::QKxShortcutInputDialog *ui;
};

#endif // QKXSHORTCUTINPUTDIALOG_H
