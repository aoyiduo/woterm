/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWOIDENTIFYDIALOG_H
#define QWOIDENTIFYDIALOG_H

#include <QDialog>
#include <QPointer>

#include "qwoglobal.h"

namespace Ui {
class QWoIdentifyDialog;
}

class QEventLoop;
class QTreeWidgetItem;

class QWoIdentifyDialog : public QDialog
{
    Q_OBJECT
private:

public:
    ~QWoIdentifyDialog();
    static QString open(bool noselect, QWidget *parent = nullptr);

protected:
    explicit QWoIdentifyDialog(bool noselect, QWidget *parent= nullptr);
    QString result() const;
private slots:
    void onButtonCreateClicked();
    void onButtonImportClicked();
    void onButtonExportClicked();
    void onButtonDeleteClicked();
    void onButtonSelectClicked();
    void onButtonRenameClicked();
    void onButtonPublicViewClicked();
    void onButtonPrivateViewClicked();
    void onItemDoubleClicked(QTreeWidgetItem*, int);

private:
    void reload();
    bool isAdministrator();
private:
    Ui::QWoIdentifyDialog *ui;
    QString m_result;
};

#endif // QWOIDENTIFYDIALOG_H
