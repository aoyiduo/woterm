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
