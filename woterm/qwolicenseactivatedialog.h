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

#ifndef QWOLICENSEACTIVATEDIALOG_H
#define QWOLICENSEACTIVATEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoLicenseActivateDialog;
}

class QWoLicenseActivateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoLicenseActivateDialog(QWidget *parent = nullptr);
    ~QWoLicenseActivateDialog();

private slots:
    void onAssistButtonClicked(int idx);
    void onActivateButtonClicked();
    void onRedeemButtonClicked();
    void onTypeButtonClicked();
    void onKeyModifyButtonClicked();
    void onRetryToRedeemLicense(int code, const QByteArray& body);
    void onValidResult(int code, const QByteArray& body);
private:
    Q_INVOKABLE void tryToRedeemLicense(const QString& code, int tryLeft);
private:
    Ui::QWoLicenseActivateDialog *ui;
};

#endif // QWOLICENSEACTIVATEDIALOG_H
