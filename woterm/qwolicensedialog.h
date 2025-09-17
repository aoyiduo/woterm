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

#ifndef QWOLICENSEDIALOG_H
#define QWOLICENSEDIALOG_H

#include <QDialog>

namespace Ui {
class QWoLicenseDialog;
}

class QWoLicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoLicenseDialog(QWidget *parent = nullptr);
    ~QWoLicenseDialog();
private slots:
    void onActivateButtonClicked();
    void onLicenseRemoveButtonClicked();
    void onVersionsDifferenceButtonClicked();
    void onTrialLinkActivated(const QString& link);
    void onSNAssistButtonClicked(int idx);
    void onMIDAssistButtonClicked(int idx);
private:
    void reset();
private:
    Ui::QWoLicenseDialog *ui;
};

#endif // QWOLICENSEDIALOG_H
