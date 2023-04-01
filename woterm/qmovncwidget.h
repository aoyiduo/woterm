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

#ifndef QMOVNCWIDGET_H
#define QMOVNCWIDGET_H

#include "qwovncwidget.h"
#include <QPointer>

class QMoVncWidget : public QWoVncWidget
{
    Q_OBJECT
public:
    explicit QMoVncWidget(const QString& target, QWidget *parent = nullptr);
    void handleKeyEvent(QKeyEvent* ev);
};

#endif // QMOVNCWIDGET_H
