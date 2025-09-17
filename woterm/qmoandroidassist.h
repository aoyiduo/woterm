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
