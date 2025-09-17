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

#ifndef QWOBASETOOLFORM_H
#define QWOBASETOOLFORM_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class QWoBaseToolForm;
}

class QMenu;
class QWoShowerWidget;

class QWoBaseToolForm : public QWidget
{
    Q_OBJECT

public:
    explicit QWoBaseToolForm(QWidget *parent = 0);
    ~QWoBaseToolForm();

    QSize sizeHint() const;
signals:
    void enter();
    void leave();
    void sizeChanged(const QSize& sz);
private slots:
    void onExitFullScreen();
    void onReturnTabWindow();
    void onFullScreen();
    void onMinimizeWindow();
    void onCloseWindow();

    void onShowerFloatChanged(QWoShowerWidget *shower, bool bFloat);
private:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    bool eventFilter(QObject *w, QEvent *e);
    void resizeEvent(QResizeEvent *e);
private:
    Q_INVOKABLE void updateStatus();
    Q_INVOKABLE void resetLayout();
private:
    Ui::QWoBaseToolForm *ui;
};

#endif // QWOBASETOOLFORM_H
