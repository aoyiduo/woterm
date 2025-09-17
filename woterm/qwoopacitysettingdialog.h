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

#ifndef QWOOPACITYSETTINGDIALOG_H
#define QWOOPACITYSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class QWoOpacitySettingDialog;
}

class QWoOpacitySettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoOpacitySettingDialog(bool turnOn, int opacity, QWidget* parent = nullptr);
    virtual ~QWoOpacitySettingDialog();

    bool opacityTurnOn();
    int opacityValue();
signals:
    void opacityChanged(bool turnOn, qreal opacity);

private slots:
    void onOpacityValueChanged();
    void onApply();
private:
    Ui::QWoOpacitySettingDialog *ui;
};

#endif // QWOOPACITYSETTINGDIALOG_H
