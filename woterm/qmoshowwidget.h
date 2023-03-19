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

#ifndef QMOSHOWWIDGET_H
#define QMOSHOWWIDGET_H

#include <QWidget>
#include <QPointer>

class QMoShowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMoShowWidget(const QString& target, QWidget *parent = nullptr);
signals:

private slots:
    void onResizeAgaint();
protected:
    QString m_target;
};

#endif // QMOSHOWWIDGET_H
