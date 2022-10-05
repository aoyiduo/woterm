/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
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
