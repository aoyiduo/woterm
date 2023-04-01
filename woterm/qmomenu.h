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

#ifndef QMOMENU_H
#define QMOMENU_H

#include <QPointer>
#include <QWidget>


namespace Ui {
class QMoMenu;
}

class QMoMenuListModel;

class QMoMenu : public QWidget
{
    Q_OBJECT

public:
    enum EMenuID {
        MID_Back = 1,
        MID_Cancel = 2,
        MID_User = 10
    };
public:
    explicit QMoMenu(QWidget *parent = nullptr);
    ~QMoMenu();
    void addItem(int mid, const QString& text, const QString& icon=QString());
    void removeItem(int mid);
signals:
    void clicked(int id);
private slots:
    void onCurrentItemClicked(const QModelIndex& idx);
private:
    virtual void showEvent(QShowEvent* e);
private:
    Ui::QMoMenu *ui;

    QPointer<QMoMenuListModel> m_model;
};

#endif // QMOMENU_H
