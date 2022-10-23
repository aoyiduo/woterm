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

#ifndef QKXPROCESSLAUNCH_H
#define QKXPROCESSLAUNCH_H

#include "qkxutil_share.h"

#include <QObject>
// replace QProcess::startDetach.
// QProcess::startDetach call system appliction will failed on windows.
// so write a new one to replace.

class KXUTIL_EXPORT QKxProcessLaunch
{
public:
    static bool startDetached(const QString &program, const QStringList &arguments=QStringList());
    static void initQtLibrary();
};

#endif // QKXPROCESSLAUNCH_H
