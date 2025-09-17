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

#ifndef QWOPLAYBOOKTERMWIDGET_H
#define QWOPLAYBOOKTERMWIDGET_H

#include <qkxtermwidget.h>
#include <QPointer>

class QWoPlaybookTermWidget : public QKxTermWidget
{
    Q_OBJECT

public:
    explicit QWoPlaybookTermWidget(QWidget *parent=nullptr);
    virtual ~QWoPlaybookTermWidget();
    Q_INVOKABLE void append(const QString& log);
    Q_INVOKABLE void scrollToEnd();
    Q_INVOKABLE void clear();
signals:
    void aboutToClose(QCloseEvent* event);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);
protected:
    void initDefault();
protected slots:
    void onResetTermSize();
private:
    void resetProperty(QVariantMap data);
};

#endif // QWOPLAYBOOKTERMWIDGET_H
