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
