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
#include "qkxmessagebox.h"
#include "qwosshconf.h"

#include <QDesktopWidget>

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
    resetProperty(mdata);
}

QVariantMap QWoSessionRDPProperty::result() const
{
    return m_result;
}

void QWoSessionRDPProperty::onButtonSaveClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    done(QDialog::Accepted);
}

void QWoSessionRDPProperty::onButtonSaveToAllClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    QWoSetting::setRdpDefault(m_result);
    QWoSshConf::instance()->removeProperties(Mstsc);
    m_result = QVariantMap();
    done(QDialog::Accepted+1);
}

QVariantMap QWoSessionRDPProperty::save()
{
    QVariantMap mvar;
    if(ui->fixRadio->isChecked()) {
        QString width = ui->fixWidth->text();
        QString height = ui->fixHeight->text();
        if(width.isEmpty() || height.isEmpty()) {
            QKxMessageBox::warning(this, tr("Parameter errors"), tr("The parameter value of width or height should be valid."));
            return QVariantMap();
        }
        mvar.insert("desktopType", "fix");
        mvar.insert("desktopWidth", width);
        mvar.insert("desktopHeight", height);
    }else{
        mvar.insert("desktopType", "desktop");
    }

    if(ui->rad16Bit->isChecked()) {
        mvar.insert("colorDepth", 16);
    }else{
        mvar.insert("colorDepth", 32);
    }
    mvar.insert("noContent", ui->chkDragContent->isChecked());
    mvar.insert("noWallpaper", ui->chkWallpaper->isChecked());
    mvar.insert("noTheme", ui->chkTheme->isChecked());
    mvar.insert("noFontSmooth", ui->chkFontSmooth->isChecked());
    mvar.insert("noSmartResize", ui->chkSmartResize->isChecked());
    mvar.insert("autoReconnect", ui->chkAutoReconnect->isChecked());
    if(ui->radPlayDisable->isChecked()) {
        mvar.insert("audioMode", 2);
    }else if(ui->radPlayLocal->isChecked()) {
        mvar.insert("audioMode", 0);
    }else{
        mvar.insert("audioMode", 1);
    }

    if(!m_bCustom) {
        QWoSetting::setRdpDefault(mvar);
    }
    return mvar;
}

void QWoSessionRDPProperty::initDefault()
{
    QVariantMap mdata = QWoSetting::rdpDefault();

    resetProperty(mdata, true);
}

void QWoSessionRDPProperty::resetProperty(const QVariantMap& mdata, bool force)
{
    if(mdata.isEmpty() && !force) {
        return;
    }
    QString deskType = mdata.value("desktopType", "desktop").toString();
    if(deskType == "desktop") {
        ui->deskRadio->setChecked(true);
        ui->fixRadio->setChecked(false);
    }else{
        ui->deskRadio->setChecked(false);
        ui->fixRadio->setChecked(true);
        QDesktopWidget desk;
        QRect rt = desk.screenGeometry(this);
        QString width = mdata.value("desktopWidth", rt.width()).toString();
        QString height = mdata.value("desktopHeight", rt.height()).toString();
        if(width.isEmpty()) {
            width = QString::number(rt.width());
        }
        if(height.isEmpty()) {
            height = QString::number(rt.height());
        }
        ui->fixWidth->setText(width);
        ui->fixHeight->setText(height);
    }

    int depth = mdata.value("colorDepth", 16).toInt();
    if(depth == 16) {
        ui->rad16Bit->setChecked(true);
    }else{
        ui->rad32Bit->setChecked(true);
    }

    int mode = mdata.value("audioMode", 0).toInt();
    if(mode == 0) {
        ui->radPlayLocal->setChecked(true);
    }else if(mode == 1) {
        ui->radPlayServer->setChecked(true);
    }else {
        ui->radPlayDisable->setChecked(true);
    }

    bool noContent = mdata.value("noContent", false).toBool();
    ui->chkDragContent->setChecked(noContent);
    bool noWallpaper = mdata.value("noWallpaper", false).toBool();
    ui->chkWallpaper->setChecked(noWallpaper);
    bool noTheme = mdata.value("noTheme", false).toBool();
    ui->chkTheme->setChecked(noTheme);
    bool noSmooth = mdata.value("noFontSmooth", true).toBool();
    ui->chkFontSmooth->setChecked(noSmooth);
    bool noResize = mdata.value("noSmartResize", true).toBool();
    ui->chkSmartResize->setChecked(noResize);
    bool autoReconnect = mdata.value("autoReconnect", false).toBool();
    ui->chkAutoReconnect->setChecked(autoReconnect);
}
