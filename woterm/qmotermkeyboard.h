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
