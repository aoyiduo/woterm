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

#ifndef QKXWALLPAPER_H
#define QKXWALLPAPER_H

#include "qkxutil_share.h"

#include <QObject>
#include <QPointer>

class QSettings;
class KXUTIL_EXPORT QKxWallPaper : public QObject
{
    Q_OBJECT
    enum EStatus {
        ES_None,
        ES_Black,
        ES_WallPaper
    };
public:
    explicit QKxWallPaper(QObject *parent = nullptr);
    ~QKxWallPaper();
    void disable(bool yes);
    void invert();
private:
    bool restore();
    bool clean();
    bool _restore();
    bool _clean();
    bool doClean(QSettings *settings);
    bool doRestore(QSettings *settings);
    bool cleanWallPaper(QSettings *settings);
    bool restoreWallPaper(QSettings *settings);
    bool cleanWindowDrag(QSettings *settings);
    bool restoreWindowDrag(QSettings *settings);

    bool isActiveProcess(int pid);
    qint64 desktopColor();
    void setDesktopColor(qint64 clr);
private:
    EStatus m_status;
};

#endif // QKXWALLPAPER_H
