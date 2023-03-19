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

#ifndef QWOAPPLICATION_H
#define QWOAPPLICATION_H

#include <QPointer>
#include <QApplication>

class QWoMainWindow;
class QWoApplication : public QApplication
{
    Q_OBJECT
public:
    explicit QWoApplication(int &argc, char **argv);
    static QWoApplication *instance();
    static QWoMainWindow *mainWindow();
signals:
private:
    Q_INVOKABLE void init();
private:
    QPointer<QWoMainWindow> m_main;
};

#endif // QWOAPPLICATION_H
