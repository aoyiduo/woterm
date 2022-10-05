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

#ifndef QKXSEARCH_H
#define QKXSEARCH_H

#include <QPointer>
#include <QWidget>

namespace Ui {
class QKxSearch;
}

class QKxTermItem;
class QKxSearch : public QWidget
{
    Q_OBJECT

public:
    explicit QKxSearch(QKxTermItem* term, QWidget *parent = 0);
    ~QKxSearch();
private slots:
    void onClose();
    void onFindPrev();
    void onFindNext();
    void onFindAll();
    void onTextChanged(const QString& txt);

private:
    Ui::QKxSearch *ui;
    QPointer<QKxTermItem> m_term;
};

#endif // QKXSEARCH_H
