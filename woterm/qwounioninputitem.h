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
