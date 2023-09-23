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

#ifndef QWOSESSIONFILEASSOCIATIONADDDIALOG_H
#define QWOSESSIONFILEASSOCIATIONADDDIALOG_H

#include <QDialog>

namespace Ui {
class QWoSessionFileAssociationAddDialog;
}

class QWoSessionFileAssociationAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSessionFileAssociationAddDialog(QWidget *parent = nullptr);
    ~QWoSessionFileAssociationAddDialog();
    void init(const QString& fileTyps, const QString& app, const QString& parameter, int priority);
    QString fileTypes() const;
    QString application() const;
    QString parameter() const;
    int priority() const;
private slots:
    void onSaveButtonClicked();
    void onAppFindButtonClicked();
private:
    Ui::QWoSessionFileAssociationAddDialog *ui;
};

#endif // QWOSESSIONFILEASSOCIATIONADDDIALOG_H
