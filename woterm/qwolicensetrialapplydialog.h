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
    Q_INVOKABLE void tryToGetLicense(int tryLeft);
private:
    Ui::QWoLicenseTrialApplyDialog *ui;
};

#endif // QWOLICENSETRIALAPPLYDIALOG_H
