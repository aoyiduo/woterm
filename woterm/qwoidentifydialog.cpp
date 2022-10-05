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

#include "qwoidentifydialog.h"
#include "ui_qwoidentifydialog.h"
#include "qwoutils.h"
#include "qwosetting.h"
#include "qworenamedialog.h"
#include "qwoidentifypublickeydialog.h"
#include "qwoidentifycreatedialog.h"
#include "qwoidentify.h"

#include <QFileDialog>
#include <QDebug>
#include <QDirModel>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QEventLoop>
#include <QCryptographicHash>
#include <QTreeWidget>

#define ROLE_IDENTIFYKEY (Qt::UserRole+2)
#define ROLE_IDENTIFYTYPE (Qt::UserRole+3)
#define ROLE_IDENTIFYFIGURE (Qt::UserRole+4)

QWoIdentifyDialog::QWoIdentifyDialog(bool noselect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoIdentifyDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);    
    ui->setupUi(this);
    setWindowTitle(tr("Identify Manage"));

    ui->btnSelect->setVisible(!noselect);
    QObject::connect(ui->btnCreate, SIGNAL(clicked()), this, SLOT(onButtonCreateClicked()));
    QObject::connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(onButtonDeleteClicked()));
    QObject::connect(ui->btnExport, SIGNAL(clicked()), this, SLOT(onButtonExportClicked()));
    QObject::connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onButtonImportClicked()));
    QObject::connect(ui->btnRename, SIGNAL(clicked()), this, SLOT(onButtonRenameClicked()));
    QObject::connect(ui->btnSelect, SIGNAL(clicked()), this, SLOT(onButtonSelectClicked()));
    QObject::connect(ui->btnView, SIGNAL(clicked()), this, SLOT(onButtonViewClicked()));
    QObject::connect(ui->identify, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
    QStringList items;
    items.append(tr("name"));
    items.append(tr("type"));
    items.append(tr("fingerprint"));
    ui->identify->setHeaderLabels(items);

    reload();
}

QWoIdentifyDialog::~QWoIdentifyDialog()
{
    delete ui;
}

QString QWoIdentifyDialog::open(bool noselect, QWidget *parent)
{
    QWoIdentifyDialog dlg(noselect, parent);
    dlg.exec();
    return dlg.result();
}


QString QWoIdentifyDialog::result() const
{
    return m_result;
}

void QWoIdentifyDialog::onButtonCreateClicked()
{
    QWoIdentifyCreateDialog dlg(this);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted) {
        reload();
    }
}

void QWoIdentifyDialog::onButtonImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    qDebug() << "fileName" << fileName;
    if(fileName.isEmpty()) {
        return;
    }
    fileName = QDir::toNativeSeparators(fileName);
    IdentifyInfo info;
    if(!QWoIdentify::infomation(fileName, &info)) {
        QMessageBox::information(this, tr("info"), tr("the identify's file is bad"));
        return;
    }

    QAbstractItemModel *model = ui->identify->model();
    for(int i = 0; i < model->rowCount(); i++) {
        QModelIndex idx = model->index(i, 0);
        QString name = idx.data().toString();
        QString figure = idx.data(ROLE_IDENTIFYFIGURE).toString();
        if(figure == info.fingureprint) {
            QMessageBox::information(this, tr("info"), tr("the same identify had been exist by name: %1").arg(name));
            return;
        }
    }

    QFile f(fileName);
    f.open(QFile::ReadOnly);
    QByteArray buf = f.readAll();
    QByteArray data = QWoUtils::fromWotermStream(buf);
    f.close();
    QString name = info.name;
    for(int i = 0; i < 10; i++) {
        QString b64Name = QWoUtils::nameToPath(name);
        QString dstFile = QWoSetting::identifyFilePath() + "/" + b64Name;
        if(!QFile::exists(dstFile)) {
            info.name = name;
            QFile dst(dstFile);
            if(dst.open(QFile::WriteOnly)) {
                dst.write(data);
            }
            dst.close();
            QStringList cols;
            cols.append(info.name);
            cols.append(info.type);
            cols.append(info.fingureprint);
            QTreeWidgetItem *item = new QTreeWidgetItem(cols);
            item->setData(0, ROLE_IDENTIFYKEY, info.key);
            item->setData(0, ROLE_IDENTIFYTYPE, info.type);
            item->setData(0, ROLE_IDENTIFYFIGURE, info.fingureprint);
            ui->identify->addTopLevelItem(item);
            break;
        }
        name = QWoRenameDialog::open(info.name, this);
        if(name.isEmpty()) {
            return;
        }
    }
}

void QWoIdentifyDialog::onButtonExportClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    QString dstFile = QWoSetting::identifyFilePath() + "/" + QWoUtils::nameToPath(name);
    QFile file(dstFile);
    if(!file.open(QFile::ReadOnly)) {
        QMessageBox::information(this, tr("info"), tr("the identify's file is not exist"));
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));
    QByteArray buf = QWoUtils::fromWotermStream(file.readAll());
    file.close();
    QFile prv(fileName);
    if(prv.open(QFile::WriteOnly)) {
        prv.write(buf);
        prv.close();

        QString type = idx2.data(ROLE_IDENTIFYTYPE).toString();
        QString key = idx2.data(ROLE_IDENTIFYKEY).toString();        
        QString content = type + " " + key + " " + name;
        QFile pub(fileName + ".pub");
        if(pub.open(QFile::WriteOnly)){
            pub.write(content.toUtf8());
            pub.close();
        }
    }
}

void QWoIdentifyDialog::onButtonDeleteClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    QDir dir(QWoSetting::identifyFilePath());
    if(!dir.remove(QWoUtils::nameToPath(name))) {
        QMessageBox::warning(this, tr("Warning"), tr("failed to delete file:%1").arg(name));
        return;
    }
    model->removeRow(idx.row());
}

void QWoIdentifyDialog::onButtonSelectClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    m_result = idx2.data().toString();
    close();
}

void QWoIdentifyDialog::onButtonRenameClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString name = idx2.data().toString();
    QString nameNew = QWoRenameDialog::open(name, this);
    if(nameNew.isEmpty() || name == nameNew) {
        return;
    }
    QDir dir(QWoSetting::identifyFilePath());

    if(!dir.rename(QWoUtils::nameToPath(name), QWoUtils::nameToPath(nameNew))) {
        QMessageBox::warning(this, tr("Warning"), tr("failed to rename file:[%1]->[%2]").arg(name).arg(nameNew));
    }
    reload();
}

void QWoIdentifyDialog::onButtonViewClicked()
{
    QModelIndex idx = ui->identify->currentIndex();
    if(!idx.isValid()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QAbstractItemModel *model = ui->identify->model();
    QModelIndex idx2 = model->index(idx.row(), 0);
    QString key = idx2.data(ROLE_IDENTIFYKEY).toString();
    if(key.isEmpty()) {
        QMessageBox::information(this, tr("info"), tr("no selection"));
        return;
    }
    QString name = idx2.data().toString();
    QString type = idx2.data(ROLE_IDENTIFYTYPE).toString();
    QString content = type + " " + key + " " + name;
    QWoIdentifyPublicKeyDialog dlg(content, this);
    dlg.exec();
}

void QWoIdentifyDialog::onItemDoubleClicked(QTreeWidgetItem *row, int col)
{
    QString name = row->data(0, Qt::DisplayRole).toString();
    if(name.isEmpty()) {
        return;
    }
    m_result = name;
    close();
}

void QWoIdentifyDialog::reload()
{
    QMap<QString, IdentifyInfo> all = QWoIdentify::all();
    ui->identify->clear();
    for(QMap<QString, IdentifyInfo>::iterator iter = all.begin(); iter != all.end(); iter++) {
        IdentifyInfo info = iter.value();
        QStringList cols;
        cols.append(info.name);
        cols.append(info.type);
        cols.append(info.fingureprint);
        QTreeWidgetItem *item = new QTreeWidgetItem(cols);
        item->setData(0, ROLE_IDENTIFYKEY, info.key);
        item->setData(0, ROLE_IDENTIFYTYPE, info.type);
        item->setData(0, ROLE_IDENTIFYFIGURE, info.fingureprint);
        QFontMetrics fm(ui->identify->font());
        QSize sz = fm.size(Qt::TextSingleLine, cols.at(0)) + QSize(50, 0);
        int csz = ui->identify->columnWidth(0);
        if(sz.width() > csz) {
            ui->identify->setColumnWidth(0, sz.width());
        }
        ui->identify->addTopLevelItem(item);
    }
}
