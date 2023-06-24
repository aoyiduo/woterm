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

#include "qwomac.h"

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
void QWoMac::closeWidget(quintptr wid)
{
    NSView *view = reinterpret_cast<NSView*>(wid);
    if(view == nil) {
        return;
    }
    NSWindow *window = view.window;
    if(window == nil) {
        return;
    }
    [window close];
}
