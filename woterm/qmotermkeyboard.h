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

#ifndef QMOTERMKEYBOARD_H
#define QMOTERMKEYBOARD_H

#include "qmokeyboard.h"

class QMoTermKeyboard : public QMoKeyboard
{
    Q_OBJECT

public:
    explicit QMoTermKeyboard(QWidget *parent = nullptr);
    virtual ~QMoTermKeyboard();

private:
};

#endif // QMOTERMKEYBOARD_H
