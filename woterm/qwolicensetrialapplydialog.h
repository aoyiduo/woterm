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

#ifndef QWOLICENSETRIALAPPLYDIALOG_H
#define QWOLICENSETRIALAPPLYDIALOG_H

#include <QDialog>

namespace Ui {
class QWoLicenseTrialApplyDialog;
}

class QWoLicenseTrialApplyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoLicenseTrialApplyDialog(QWidget *parent = nullptr);
    ~QWoLicenseTrialApplyDialog();

private slots:
    void onRetryButtonClicked();
    void onRetryToGetLicense(int code, const QByteArray& body);
private:
    Ui::QWoLicenseTrialApplyDialog *ui;
};

#endif // QWOLICENSETRIALAPPLYDIALOG_H
