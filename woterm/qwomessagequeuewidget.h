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

#ifndef QWOMESSAGEQUEUEWIDGET_H
#define QWOMESSAGEQUEUEWIDGET_H

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

#include <QWidget>
#include <QList>
#include <QPointer>

namespace Ui {
class QWoMessageQueueWidget;
}


class QWoMessageQueueModel;
class QWoMessageQueueWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWoMessageQueueWidget(QWidget *parent = nullptr);
    virtual ~QWoMessageQueueWidget();

    void warning(const QString& title, const QString& content);
private:
    Q_INVOKABLE void init();
private:
    Ui::QWoMessageQueueWidget *ui;
    QPointer<QWoMessageQueueModel> m_model;
};

#endif // QWOMESSAGEQUEUEWIDGET_H
