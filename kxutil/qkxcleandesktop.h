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

#ifndef QKXCLEANDESKTOP_H
#define QKXCLEANDESKTOP_H

#include <QObject>

class QKxCleanDesktop : public QObject
{
    Q_OBJECT
public:
    explicit QKxCleanDesktop(QObject *parent = nullptr);
    virtual ~QKxCleanDesktop();
public slots:
    void backupDesktopColor();
    void setDesktopColor();
    void backupWallpaper();
    void disableWallpaper();
    void enableWallpaper();
    void disableWallpaper2();
    void enableWallpaper2();

    void disableEffects();
    void enableEffects();
signals:

public slots:
private:
    bool restoreActiveDesktop;
    bool restoreWallpaper;
    bool restorePattern;
    bool restoreEffects;
    bool uiEffects;
    bool comboBoxAnim, gradientCaptions, hotTracking, listBoxSmoothScroll, menuAnim;
};

#endif // QKXCLEANDESKTOP_H
