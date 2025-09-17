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

#ifndef QWOSESSIONRDPPROPERTY_H
#define QWOSESSIONRDPPROPERTY_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class QWoSessionRDPProperty;
}

class QWoSessionRDPProperty : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionRDPProperty(QWidget *parent = nullptr);
    ~QWoSessionRDPProperty();
    void setCustom(const QVariantMap& prop);
    QVariantMap result() const;
private slots:
    void onButtonSaveClicked();
    void onButtonSaveToAllClicked();
private:
    QVariantMap save();
    void initDefault();
    void resetProperty(const QVariantMap& dm, bool force = false);
private:
    Ui::QWoSessionRDPProperty *ui;
    bool m_bCustom;
    QVariantMap m_result;
};

#endif // QWOSESSIONRDPPROPERTY_H
