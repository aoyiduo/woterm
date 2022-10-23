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

#include <QApplication>

class QWoApplication : public QApplication
{
    Q_OBJECT
public:
    explicit QWoApplication(int &argc, char **argv);

signals:

};

#endif // QWOAPPLICATION_H
