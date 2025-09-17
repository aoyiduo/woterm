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

#ifndef QWOUNIONINPUTITEM_H
#define QWOUNIONINPUTITEM_H

#include <QWidget>

namespace Ui {
class QWoUnionInputItem;
}

class QWoUnionInputItem : public QWidget
{
    Q_OBJECT

public:
    explicit QWoUnionInputItem(QWidget *parent = nullptr);
    virtual ~QWoUnionInputItem();

signals:
    void inputArrived(const QString& cmd);
private slots:
    void onEditFinished();
    void onSettingsButtonClicked();
private:
    Ui::QWoUnionInputItem *ui;
    bool m_isEnterAsRN;
};

#endif // QWOUNIONINPUTITEM_H
