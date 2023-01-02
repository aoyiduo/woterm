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
private:
    void initDefault();

private:
    Ui::QWoSessionVNCProperty *ui;
    bool m_bCustom;
    QVariantMap m_result;
};

#endif // QWOSESSIONVNCPROPERTY_H
