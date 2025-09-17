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

#ifndef QWOHOSTINFOEDIT_H
#define QWOHOSTINFOEDIT_H

#include "qwoglobal.h"

#include <QDialog>

namespace Ui {
class QWoHostInfo;
}

class QWoHostInfoEdit : public QDialog
{
    Q_OBJECT

public:
    explicit QWoHostInfoEdit(QWidget *parent = nullptr);
    explicit QWoHostInfoEdit(const QString& name, QWidget *parent = nullptr);
    ~QWoHostInfoEdit();

private slots:
    void onAuthCurrentIndexChanged(const QString & txt);
    void onButtonSaveClicked();
    void onButtonJumpBrowserClicked();
    void onButtonIdentifyBrowserClicked();
private:
    void init();
private:
    QString m_name;
    Ui::QWoHostInfo *ui;
};

#endif // QWOHOSTINFOEDIT_H
