/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWOSETTINGDIALOG_H
#define QWOSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSettingDialog;
}

class QWoSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSettingDialog(QWidget *parent = nullptr);
    ~QWoSettingDialog();

private slots:
    void onDesktopEntryPressed();
private:
    Ui::QWoSettingDialog *ui;
};

#endif // QWOSETTINGDIALOG_H
