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

#include "qwosettingdialog.h"
#include "ui_qwosettingdialog.h"


#include "qwoutils.h"

#include <QDir>

QWoSettingDialog::QWoSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSettingDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    int err = QWoUtils::hasDesktopShortcut(false);
    ui->deskEntry->setChecked(err > 0);
    QObject::connect(ui->deskEntry, SIGNAL(pressed()), SLOT(onDesktopEntryPressed()));
}

QWoSettingDialog::~QWoSettingDialog()
{
    delete ui;
}

void QWoSettingDialog::onDesktopEntryPressed()
{
    if(ui->deskEntry->isChecked()) {
        QWoUtils::removeDesktopShortcut(false);
    }else{
        if(QWoUtils::hasDesktopShortcut(false) > 0) {
            return;
        }
        QString icon = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../woterm.png");
        QWoUtils::createDesktopShortcut(false, "WoTerm", "WoTerm Remote Assistant", icon);
    }
}
