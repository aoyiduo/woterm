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

#ifndef QWOTUNNELMODIFYDIALOG_H
#define QWOTUNNELMODIFYDIALOG_H

#include "qwotunneladddialog.h"

class QWoTunnelModifyDialog : public QWoTunnelAddDialog
{
public:
    explicit QWoTunnelModifyDialog(const TunnelInfo& ti, QWidget *parent = nullptr);
};

#endif // QWOTUNNELMODIFYDIALOG_H
