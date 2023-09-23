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
