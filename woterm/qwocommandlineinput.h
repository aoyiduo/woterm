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

#ifndef QWOCOMMANDLINEINPUT_H
#define QWOCOMMANDLINEINPUT_H

#include <QWidget>

namespace Ui {
class QWoCommandLineInput;
}

class QWoCommandLineInput : public QWidget
{
    Q_OBJECT

public:
    explicit QWoCommandLineInput(QWidget *parent = nullptr);
    ~QWoCommandLineInput();
signals:
    void returnPressed(const QString& cmd);

private slots:
    void onInputReturnPressed();
    void onCloseButtonClicked();

private:
    Ui::QWoCommandLineInput *ui;
};

#endif // QWOCOMMANDLINEINPUT_H
