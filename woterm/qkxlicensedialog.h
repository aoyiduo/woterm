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

#ifndef QKXLICENSEDIALOG_H
#define QKXLICENSEDIALOG_H

#include <QDialog>

namespace Ui {
class QKxLicenseDialog;
}

class QKxLicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QKxLicenseDialog(QWidget *parent = nullptr);
    ~QKxLicenseDialog();
private slots:
    void onActivateButtonClicked();
private:
    Ui::QKxLicenseDialog *ui;
};

#endif // QKXLICENSEDIALOG_H
