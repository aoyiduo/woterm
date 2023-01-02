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

#include "qwosystemoptiondialog.h"
#include "ui_qwosystemoptiondialog.h"

#include "qwosetting.h"

#include <QStringListModel>
#include <QComboBox>

QWoSystemOptionDialog::QWoSystemOptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSystemOptionDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("System options"));

    m_langs = QWoSetting::allLanguages();
    ui->langChooser->setModel(new QStringListModel(m_langs.keys(), this));
    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    QString lang = QWoSetting::languageFile();
    if(m_langs.values().contains(lang)) {
        ui->langChooser->setCurrentText(QWoSetting::languageName(lang));
    }
    adjustSize();
}

QWoSystemOptionDialog::~QWoSystemOptionDialog()
{
    delete ui;
}

QString QWoSystemOptionDialog::language() const
{
    QString lang = ui->langChooser->currentText();
    QString path = m_langs.value(lang);
    if(path.isEmpty()) {
        return QString();
    }
    return m_langs.value(lang);
}

void QWoSystemOptionDialog::onButtonSaveClicked()
{
    done(QDialog::Accepted);
}
