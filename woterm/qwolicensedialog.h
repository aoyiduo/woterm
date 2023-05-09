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
