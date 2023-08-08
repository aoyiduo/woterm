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
