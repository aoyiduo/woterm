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

#ifndef QWOTUNNELMODIFYDIALOG_H
#define QWOTUNNELMODIFYDIALOG_H

#include "qwotunneladddialog.h"

class QWoTunnelModifyDialog : public QWoTunnelAddDialog
{
public:
    explicit QWoTunnelModifyDialog(const TunnelInfo& ti, QWidget *parent = nullptr);
};

#endif // QWOTUNNELMODIFYDIALOG_H
