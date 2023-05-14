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

#ifndef QWOMESSAGEQUEUEWIDGET_H
#define QWOMESSAGEQUEUEWIDGET_H

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
