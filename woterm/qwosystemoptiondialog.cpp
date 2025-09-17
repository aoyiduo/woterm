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

#include "qwosystemoptiondialog.h"
#include "ui_qwosystemoptiondialog.h"

#include "qwosetting.h"
#include "qwotheme.h"

#include <QStringListModel>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>

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
    QString lang = QWoSetting::languageFileName();

    if(m_langs.values().contains(lang)) {
        QString name = m_langs.key(lang);
        ui->langChooser->setCurrentText(name);
    }

    QWoTheme *theme = QWoTheme::instance();
    m_skins = theme->skinFriendNames();
    ui->skinChooser->setModel(new QStringListModel(m_skins.values(), this));
    QString uname = theme->skinUniqueName();
    qDebug() << "QWoSystemOptionDialog" << uname << m_skins;
    if(m_skins.contains(uname)) {
        QString friendName = m_skins.value(uname);
        ui->skinChooser->setCurrentText(friendName);
    }

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->skinChooser, SIGNAL(currentIndexChanged(QString)), this, SLOT(onSkinCurrentIndexChanged(QString)));

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
    return path;
}

QString QWoSystemOptionDialog::skin() const
{
    QString name = ui->skinChooser->currentText();
    QString path = m_skins.key(name);
    if(path.isEmpty()) {
        return QString();
    }
    return path;
}

void QWoSystemOptionDialog::onButtonSaveClicked()
{
    done(QDialog::Accepted);
}

void QWoSystemOptionDialog::onSkinCurrentIndexChanged(const QString &name)
{
    QWoTheme *theme = QWoTheme::instance();
    for(auto it = m_skins.begin(); it != m_skins.end(); it++) {
        if(it.value() == name) {
            return;
        }
    }
}
