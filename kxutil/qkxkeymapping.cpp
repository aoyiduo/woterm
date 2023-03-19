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

#include "qkxkeymapping.h"

QKxKeyMapping::QKxKeyMapping(QObject *parent) : QObject(parent)
{

}

Qt::Key QKxKeyMapping::CharToKey(QChar qc)
{
    char c = qc.toLatin1();
    if(c >= 'a' && c <= 'z') {
        return Qt::Key(Qt::Key_A + c - 'a');
    }
    if(c >= 'A' && c <= 'Z') {
        return Qt::Key(Qt::Key_A + c - 'A');
    }
    if(c >= '0' && c <= '9') {
        return Qt::Key(Qt::Key_0 + c - '0');
    }
    // symbol's unicode value are equal to Qt::Key;
    return Qt::Key(qc.unicode());
}
