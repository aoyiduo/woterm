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

#include "qwosessionvncproperty.h"
#include "ui_qwosessionvncproperty.h"

#include "qwoutils.h"
#include "qwosetting.h"
#include "qwosshconf.h"

#include <QStringListModel>

QWoSessionVNCProperty::QWoSessionVNCProperty(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QWoSessionVNCProperty)
    , m_bCustom(false)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("VNC Properties"));
    QStringList rfbs;
    rfbs << "RFB_33" << "RFB_37" << "RFB_38";
    ui->proto->setModel(new QStringListModel(rfbs, this));
    ui->proto->setCurrentText("RFB_38");
    QStringList fmts;
    fmts << "RGB32_888" << "RGB16_565" << "RGB15_555" << "RGB8_332" << "RGB8_Map";
    ui->pixel->setModel(new QStringListModel(fmts, this));
    ui->pixel->setCurrentText("RGB16_565");
    initDefault();

    QObject::connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onButtonSaveClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));

    adjustSize();
}

QWoSessionVNCProperty::~QWoSessionVNCProperty()
{
    delete ui;
}

void QWoSessionVNCProperty::setCustom(const QVariantMap &mdata)
{
    m_bCustom = true;
    if(!mdata.isEmpty()) {
        {
            QString v = mdata.value("vncProto", "RFB_38").toString();
            if(v == "RFB_33" || v == "RFB_37" || v == "RFB_38") {
                ui->proto->setCurrentText(v);
            }else {
                ui->proto->setCurrentText("RFB_38");
            }
        }
        {
            QString v = mdata.value("vncPixel", "RGB16_565").toString();
            if(v == "RGB32_888" || v == "RGB16_565"
                    || v == "RGB15_555" || v == "RGB8_332"
                    || v == "RGB8_Map") {
                ui->pixel->setCurrentText(v);
            }else {
                ui->pixel->setCurrentText("RGB16_565");
            }
        }
        ui->h264->setChecked(mdata.value("vncH264", true).toBool());
        ui->jpeg->setChecked(mdata.value("vncJPEG", true).toBool());
        ui->zrle3->setChecked(mdata.value("vncZRLE3", true).toBool());
        ui->trle3->setChecked(mdata.value("vncTRLE3", true).toBool());
        ui->zrle2->setChecked(mdata.value("vncZRLE2", true).toBool());
        ui->trle2->setChecked(mdata.value("vncTRLE2", true).toBool());
        ui->zrle->setChecked(mdata.value("vncZRLE", true).toBool());
        ui->trle->setChecked(mdata.value("vncTRLE", true).toBool());
        ui->hextile->setChecked(mdata.value("vncHextile", true).toBool());
        ui->rre->setChecked(mdata.value("vncRRE", true).toBool());
        ui->copyrect->setChecked(mdata.value("vncCopyRect", true).toBool());
        ui->raw->setChecked(/*mdata.value("vncRaw", true).toBool()*/true);
        ui->deskresize->setChecked(mdata.value("vncDeskResize", true).toBool());
        ui->viewonly->setChecked(mdata.value("vncViewOnly", false).toBool());
    }
}

QVariantMap QWoSessionVNCProperty::result() const
{
    return m_result;
}

void QWoSessionVNCProperty::onButtonSaveClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    done(QDialog::Accepted);
}

void QWoSessionVNCProperty::onButtonSaveToAllClicked()
{
    m_result = save();
    if(m_result.isEmpty()) {
        return;
    }
    QWoSetting::setVncDefault(m_result);
    QWoSshConf::instance()->removeProperties(Vnc);
    m_result = QVariantMap();
    done(QDialog::Accepted+1);
}

void QWoSessionVNCProperty::initDefault()
{
    QVariantMap mdata = QWoSetting::vncDefault();

    {QString v = mdata.value("vncProto", "RFB_38").toString();
    if(v == "RFB_33" || v == "RFB_37" || v == "RFB_38") {
        ui->proto->setCurrentText(v);
    }else {
        ui->proto->setCurrentText("RFB_38");
    }}
    {QString v = mdata.value("vncPixel", "RGB16_565").toString();
    if(v == "RGB32_888" || v == "RGB16_565"
            || v == "RGB15_555" || v == "RGB8_332"
            || v == "RGB8_Map") {
        ui->pixel->setCurrentText(v);
    }else {
        ui->pixel->setCurrentText("RGB16_565");
    }}

    ui->h264->setChecked(mdata.value("vncH264", true).toBool());
    ui->jpeg->setChecked(mdata.value("vncJPEG", true).toBool());
    ui->zrle3->setChecked(mdata.value("vncZRLE3", true).toBool());
    ui->trle3->setChecked(mdata.value("vncTRLE3", true).toBool());
    ui->zrle2->setChecked(mdata.value("vncZRLE2", true).toBool());
    ui->trle2->setChecked(mdata.value("vncTRLE2", true).toBool());
    ui->zrle->setChecked(mdata.value("vncZRLE", true).toBool());
    ui->trle->setChecked(mdata.value("vncTRLE", true).toBool());
    ui->hextile->setChecked(mdata.value("vncHextile", true).toBool());
    ui->rre->setChecked(mdata.value("vncRRE", true).toBool());
    ui->copyrect->setChecked(mdata.value("vncCopyRect", true).toBool());
    ui->raw->setChecked(/*mdata.value("vncRaw", true).toBool()*/true);
    ui->deskresize->setChecked(mdata.value("vncDeskResize", true).toBool());
    ui->viewonly->setChecked(mdata.value("vncViewOnly", false).toBool());
}

QVariantMap QWoSessionVNCProperty::save()
{
    QVariantMap mvar;
    QString proto = ui->proto->currentText();
    mvar.insert("vncProto", proto);
    QString pixel = ui->pixel->currentText();
    mvar.insert("vncPixel", pixel);
    bool zrle = ui->zrle->isChecked();
    mvar.insert("vncZRLE", zrle);
    bool trle = ui->trle->isChecked();
    mvar.insert("vncTRLE", trle);
    bool zrle2 = ui->zrle2->isChecked();
    mvar.insert("vncZRLE2", zrle2);
    bool trle2 = ui->trle2->isChecked();
    mvar.insert("vncTRLE2", trle2);
    bool zrle3 = ui->zrle3->isChecked();
    mvar.insert("vncZRLE3", zrle3);
    bool trle3 = ui->trle3->isChecked();
    mvar.insert("vncTRLE3", trle3);
    bool h264 = ui->h264->isChecked();
    mvar.insert("vncH264", h264);
    bool jpeg = ui->jpeg->isChecked();
    mvar.insert("vncJPEG", jpeg);
    bool hextile = ui->hextile->isChecked();
    mvar.insert("vncHextile", hextile);
    bool copyrect = ui->copyrect->isChecked();
    mvar.insert("vncCopyRect", copyrect);
    bool rre = ui->rre->isChecked();
    mvar.insert("vncRRE", rre);
    bool raw = ui->rre->isChecked();
    mvar.insert("vncRaw", raw);
    bool deskresize = ui->deskresize->isChecked();
    mvar.insert("vncDeskResize", deskresize);
    bool viewonly = ui->viewonly->isChecked();
    mvar.insert("vncViewOnly", viewonly);
    if(!m_bCustom) {
        QWoSetting::setVncDefault(mvar);
    }
    return mvar;
}
