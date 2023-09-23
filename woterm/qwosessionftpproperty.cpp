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

#include "qwosessionftpproperty.h"
#include "ui_qwosessionftpproperty.h"
#include "qkxmessagebox.h"
#include "qwosetting.h"
#include "qwosessionfileassociationmodel.h"
#include "qwosessionfileassociationadddialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QVariantMap>
#include <QStandardItemModel>

QWoSessionFtpProperty::QWoSessionFtpProperty(bool editAsk, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoSessionFtpProperty)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("File association"));

    ui->btnUseIt->setVisible(editAsk);


    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));
    QObject::connect(ui->btnModify, SIGNAL(clicked()), this, SLOT(onModifyButtonClicked()));
    QObject::connect(ui->btnRemove, SIGNAL(clicked()), this, SLOT(onRemoveButtonClicked()));
    QObject::connect(ui->types, &QTreeView::clicked, this, [=](const QModelIndex& idx){
        ui->btnModify->setVisible(idx.isValid());
        ui->btnRemove->setVisible(idx.isValid());
    });
    ui->btnModify->setVisible(false);
    ui->btnRemove->setVisible(false);



    QObject::connect(ui->btnUseIt, SIGNAL(clicked()), this, SLOT(onUseItButtonClicked()));

    m_model = QWoSessionFileAssociationModel::instance();
    ui->types->setModel(m_model);
    QObject::connect((QWoSessionFileAssociationModel*)m_model, &QWoSessionFileAssociationModel::modelReset, this, [=](){
        ui->btnModify->setVisible(false);
        ui->btnRemove->setVisible(false);
    });

    ui->chkDefault->setChecked(m_model->autoAddDefaultEditor());
    QObject::connect(ui->chkDefault, &QCheckBox::clicked, this, [=](){
        m_model->setAutoAddDefaultEditor(ui->chkDefault->isChecked());
    });

    adjustSize();
}

QWoSessionFtpProperty::~QWoSessionFtpProperty()
{
    delete ui;
}

void QWoSessionFtpProperty::onAddButtonClicked()
{
    QWoSessionFileAssociationAddDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_model->add(dlg.fileTypes(), dlg.application(), dlg.parameter(), dlg.priority());
    }
}

void QWoSessionFtpProperty::onModifyButtonClicked()
{
    QModelIndex idx = ui->types->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    QWoSessionFileAssociationAddDialog dlg(this);
    const FileAssociation& fa = m_model->data(idx, ROLE_INDEX).value<FileAssociation>();
    dlg.init(fa.type, fa.application, fa.parameter, fa.priority);
    if(dlg.exec() == QDialog::Accepted+1) {
        m_model->update(idx.row(), dlg.fileTypes(), dlg.application(), dlg.parameter(), dlg.priority());
    }
}

void QWoSessionFtpProperty::onRemoveButtonClicked()
{
    QModelIndex idx = ui->types->currentIndex();
    if(!idx.isValid()) {
        return;
    }
    m_model->remove(idx.row());
}

void QWoSessionFtpProperty::onUseItButtonClicked()
{
    QModelIndex idx = ui->types->currentIndex();
    if(!idx.isValid()) {
        QKxMessageBox::information(this, tr("Application error"), tr("Select an item to apply"));
        return;
    }
    const FileAssociation& fa = m_model->data(idx, ROLE_INDEX).value<FileAssociation>();
    QFileInfo fi(fa.application);
    if(!fi.isExecutable()) {
        QKxMessageBox::information(this, tr("Application error"), tr("It is not a valid executable program"));
        return;
    }
    emit useIt(fa.application, fa.parameter);
    done(QDialog::Accepted+1);
}
