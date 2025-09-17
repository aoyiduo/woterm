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
