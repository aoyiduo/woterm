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

#include "qwosessionrdpproperty.h"
#include "ui_qwosessionrdpproperty.h"

#include "qwoutils.h"
#include "qwosetting.h"

QWoSessionRDPProperty::QWoSessionRDPProperty(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSessionRDPProperty)
    , m_bCustom(false)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("RDP Properties"));

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    initDefault();

    adjustSize();
}

QWoSessionRDPProperty::~QWoSessionRDPProperty()
{
    delete ui;
}

void QWoSessionRDPProperty::setCustom(const QVariantMap &mdata)
{
    m_bCustom = true;
    if(!mdata.isEmpty()) {
        if(mdata.contains("desktopType")) {
            QString deskType = mdata.value("desktopType", "desktop").toString();
            if(deskType == "desktop") {
                ui->deskRadio->setChecked(true);
                ui->fixRadio->setChecked(false);
            }else{
                ui->deskRadio->setChecked(false);
                ui->fixRadio->setChecked(true);
                QString width = mdata.value("desktopWidth", "1024").toString();
                QString height = mdata.value("desktopHeight", "768").toString();
                ui->fixWidth->setText(width);
                ui->fixHeight->setText(height);
            }
        }
    }
}

QVariantMap QWoSessionRDPProperty::result() const
{
    return m_result;
}

void QWoSessionRDPProperty::onButtonSaveClicked()
{
    QVariantMap mvar;    
    if(ui->fixRadio->isChecked()) {
        QString width = ui->fixWidth->text();
        QString height = ui->fixHeight->text();
        mvar.insert("desktopType", "fix");
        mvar.insert("desktopWidth", width);
        mvar.insert("desktopHeight", height);
    }else{
        mvar.insert("desktopType", "desktop");
    }

    m_result = mvar;
    if(!m_bCustom) {
        QWoSetting::setRdpDefault(mvar);
    }
    close();
}

void QWoSessionRDPProperty::initDefault()
{
    QVariantMap mdata = QWoSetting::rdpDefault();

    QString deskType = mdata.value("desktopType", "desktop").toString();
    if(deskType == "desktop") {
        ui->deskRadio->setChecked(true);
        ui->fixRadio->setChecked(false);
    }else{
        ui->deskRadio->setChecked(false);
        ui->fixRadio->setChecked(true);
        QString width = mdata.value("desktopWidth", "1024").toString();
        QString height = mdata.value("desktopHeight", "768").toString();
        ui->fixWidth->setText(width);
        ui->fixHeight->setText(height);
    }
}
