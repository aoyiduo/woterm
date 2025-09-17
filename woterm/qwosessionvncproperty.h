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

#ifndef QWOSESSIONVNCPROPERTY_H
#define QWOSESSIONVNCPROPERTY_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class QWoSessionVNCProperty;
}

class QWoSessionVNCProperty : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionVNCProperty(QWidget *parent = nullptr);
    ~QWoSessionVNCProperty();

    void setCustom(const QVariantMap& prop);
    QVariantMap result() const;
private slots:
    void onButtonSaveClicked();
    void onButtonSaveToAllClicked();
private:
    void initDefault();
    QVariantMap save();
private:
    Ui::QWoSessionVNCProperty *ui;
    bool m_bCustom;
    QVariantMap m_result;
};

#endif // QWOSESSIONVNCPROPERTY_H
