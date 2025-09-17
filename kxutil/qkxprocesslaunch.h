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
