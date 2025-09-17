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

#ifndef QWOPLAYBOOKCONSOLE_H
#define QWOPLAYBOOKCONSOLE_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class QWoPlaybookConsole;
}

class QShowEvent;
class QHideEvent;
class QTimer;

class QWoPlaybookConsole : public QWidget
{
    Q_OBJECT

public:
    explicit QWoPlaybookConsole(const QString& name, QWidget *parent = nullptr);
    ~QWoPlaybookConsole();

    void append(const QString& msg);
    int maximumBlockCount() const;
    void setMaximumBlockCount(int cnt);
private slots:
    void onMaxBlockArrived();
protected:
    virtual void showEvent(QShowEvent *ev);
    virtual void hideEvent(QHideEvent *ev);
private:
    Ui::QWoPlaybookConsole *ui;
};

#endif // QWOPLAYBOOKCONSOLE_H
