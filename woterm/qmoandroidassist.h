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

#ifndef QMOANDROIDASSIST_H
#define QMOANDROIDASSIST_H

#include <QObject>

class QMoAndroidAssist : public QObject
{
    Q_OBJECT
public:
    explicit QMoAndroidAssist(QObject *parent = nullptr);
    static QMoAndroidAssist *instance();
    Q_INVOKABLE void notify(const QString& title, const QString& msg);
    Q_INVOKABLE int installAPK(const QString& fileApk);
    void onMessageFromJava(const QByteArray& msg);
private:
    void init();
};

#endif // QMOANDROIDASSIST_H
