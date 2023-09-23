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

#ifndef QWOSESSIONFTPPROPERTY_H
#define QWOSESSIONFTPPROPERTY_H

#include <QDialog>
#include <QList>
#include <QPointer>

namespace Ui {
class QWoSessionFtpProperty;
}

class QWoSessionFileAssociationModel;
class QWoSessionFtpProperty : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionFtpProperty(bool editAsk, QWidget *parent = nullptr);
    ~QWoSessionFtpProperty();

signals:
    void useIt(const QString& appliction, const QString& param);
private slots:
    void onAddButtonClicked();
    void onModifyButtonClicked();
    void onRemoveButtonClicked();
    void onUseItButtonClicked();

private:
    Ui::QWoSessionFtpProperty *ui;
    QPointer<QWoSessionFileAssociationModel> m_model;
};

#endif // QWOSESSIONFTPPROPERTY_H
