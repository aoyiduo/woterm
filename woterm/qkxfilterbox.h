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

#ifndef QKXFILTERBOX_H
#define QKXFILTERBOX_H

#include <QComboBox>
#include <QPointer>

class QWoHostListModel;
class QSortFilterProxyModel;
class QLineEdit;
class QKxFilterBox : public QComboBox
{
    Q_OBJECT
public:
    explicit QKxFilterBox(QWidget *parent);
    virtual ~QKxFilterBox();
private slots:
    void onEditReturnPressed();
    void onEditTextChanged(const QString& txt);
private:
    QPointer<QSortFilterProxyModel> m_proxyModel;
    QPointer<QLineEdit> m_input;
};

#endif // QKXFILTERBOX_H
