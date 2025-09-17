/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
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
