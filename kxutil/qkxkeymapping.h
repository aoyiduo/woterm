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

#ifndef QKXKEYMAPPING_H
#define QKXKEYMAPPING_H

#include "qkxutil_share.h"

#include <QObject>

class KXUTIL_EXPORT QKxKeyMapping : public QObject
{
    Q_OBJECT
public:
    explicit QKxKeyMapping(QObject *parent = nullptr);
    static Qt::Key CharToKey(QChar c);
};

#endif // QKXKEYMAPPING_H
