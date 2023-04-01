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

#ifndef QMOAPPLICATION_H
#define QMOAPPLICATION_H

#include <QApplication>
#include <QPointer>

class QMoMainWindow;
class QMoRecentHistoryModel;
class QMoApplication : public QApplication
{
    Q_OBJECT
public:
    explicit QMoApplication(int &argc, char **argv);
    static QMoApplication *instance();
    static QMoMainWindow *mainWindow();
    static qint64 elapse();

    Q_INVOKABLE static void qmlCopyText(const QString& txt);
signals:

private:
    Q_INVOKABLE void init();
private:    
    QPointer<QMoMainWindow> m_main;
    qint64 m_timeStart;
};

#endif // QMOAPPLICATION_H
