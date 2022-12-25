/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#ifndef QKXBUTTONASSIST_H
#define QKXBUTTONASSIST_H

#include <QPointer>
#include <QObject>
#include <QList>

class QHBoxLayout;
class QToolButton;
class QKxButtonAssist : public QObject
{
    Q_OBJECT
public:
    explicit QKxButtonAssist(const QString& icon, bool frame, QWidget *parent = nullptr);
    explicit QKxButtonAssist(const QString& icon, QWidget *parent = nullptr);
    QToolButton *append(const QString &icon, bool frame);
    QToolButton *button(int idx);
    void appendSeperator();
    void setEnabled(int idx, bool on);
    bool isEnabled(int idx);
    void setIconMargins(int m);
signals:
    void clicked(int idx);
    void pressed(QToolButton* btn);
public slots:
    void onClicked();
private:
    QPointer<QHBoxLayout> m_layout;
    QList<QPointer<QToolButton>> m_btns;
};

#endif // QKXBUTTONASSIST_H
