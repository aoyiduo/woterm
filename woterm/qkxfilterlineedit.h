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

#ifndef QKXFILTERLINEEDIT_H
#define QKXFILTERLINEEDIT_H

#include <QPointer>
#include <QLineEdit>

class QKxFilterListView;
class QKxFilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit QKxFilterLineEdit(QWidget *parent);
    virtual ~QKxFilterLineEdit();
signals:
    void editArrived(const QString& name);
    void targetArrived(const QString& name, int type);
    void createArrived(const QString& name);
private slots:
    void onListViewItemClicked(const QString& name, int type);
    void onAssistButtonClicked(int idx);
    void onAssistReturnPressed();
private:
    virtual void keyPressEvent(QKeyEvent *ev);
private:
    QPointer<QKxFilterListView> m_listView;
};

#endif // QKXFILTERLINEEDIT_H
