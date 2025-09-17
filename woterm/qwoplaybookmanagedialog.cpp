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

#include "qwoplaybookmanagedialog.h"
#include "ui_qwoplaybookmanagedialog.h"

#include "qwosetting.h"
#include "qkxmessagebox.h"
#include "qwoutils.h"
#include "qwoplaybookconfiguredialog.h"

#include <QUrl>
#include <QDir>
#include <QStringListModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>

QWoPlaybookManageDialog::QWoPlaybookManageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QWoPlaybookManageDialog)
{
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags &~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    setWindowTitle(tr("Playbooks"));
    ui->playbooks->setModel(new QStringListModel(ui->playbooks));
    ui->playbooks->setEditTriggers(QListView::NoEditTriggers);

    QObject::connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
    QObject::connect(ui->btnBrowser, SIGNAL(clicked()), this, SLOT(onBrowserButtonClicked()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->btnConfigure, SIGNAL(clicked()), this, SLOT(onConfigureButtonClicked()));
    QObject::connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(onCopyButtonClicked()));
    QObject::connect(ui->btnRefresh, SIGNAL(clicked()), this, SLOT(onRefreshButtonClicked()));
    QObject::connect(ui->btnRemove, SIGNAL(clicked()), this, SLOT(onRemoveButtonClicked()));
    QObject::connect(ui->playbooks, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onPlaybooksItemDbClicked(QModelIndex)));

    QMetaObject::invokeMethod(this, "reload", Qt::QueuedConnection);
    adjustSize();
}

QWoPlaybookManageDialog::~QWoPlaybookManageDialog()
{
    delete ui;
}

QString QWoPlaybookManageDialog::name() const
{
    return itemToName(ui->playbooks->currentIndex());
}

QString QWoPlaybookManageDialog::path() const
{
    return itemToPath(ui->playbooks->currentIndex());
}

void QWoPlaybookManageDialog::onApplyButtonClicked()
{
    QString path = itemToPath(ui->playbooks->currentIndex());
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("No item was selected to continue."));
        return;
    }
    done(QDialog::Accepted+1);
}

void QWoPlaybookManageDialog::onBrowserButtonClicked()
{
    QString path = itemToPath(ui->playbooks->currentIndex());
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("No item was selected to continue."));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void QWoPlaybookManageDialog::onConfigureButtonClicked()
{
    QWoPlaybookConfigureDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted+1) {
        QWoSetting::setCustomPlaybooksPath(dlg.path());
    }
}

void QWoPlaybookManageDialog::onCopyButtonClicked()
{
    QString path = itemToPath(ui->playbooks->currentIndex());
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("No item was selected to continue."));
        return;
    }
    QString pathDst = QWoSetting::customPlaybooksPath();
    if(pathDst.isEmpty()) {
        QMetaObject::invokeMethod(this, "onConfigureButtonClicked", Qt::QueuedConnection);
        return;
    }
    QFileInfo fi(path);
    QString pathTarget = pathDst + "/" + fi.fileName();
    QWoUtils::removeDirectory(pathTarget);
    QWoUtils::copyDirectory(path, pathTarget);
    QString jsFile = pathTarget + "/playbook.json";
    QFile f(jsFile);
    if(!f.open(QFile::ReadOnly)) {
        return;
    }
    QJsonDocument jsdoc = QJsonDocument::fromJson(f.readAll());
    if(!jsdoc.isObject()) {
        return;
    }
    QJsonObject obj = jsdoc.object();
    obj["name"] = obj["name"].toString() + "_Copy";
    jsdoc.setObject(obj);
    QByteArray data = jsdoc.toJson(QJsonDocument::Indented);
    f.close();
    if(f.open(QFile::WriteOnly)) {
        f.write(data);
    }
    f.close();
    reload();
}

void QWoPlaybookManageDialog::onRefreshButtonClicked()
{
    reload();
}

void QWoPlaybookManageDialog::onRemoveButtonClicked()
{
    QString path = itemToPath(ui->playbooks->currentIndex());
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("No item was selected to continue."));
        return;
    }
    QString pathPrv = QWoSetting::playbooksPath();
    if(path.startsWith(pathPrv)) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("Unable to delete official example."));
        return;
    }
    if(QKxMessageBox::warning(this, tr("Playbook information"), tr("it cannot be restored after permanent deletion, contiue to do or not?"), QMessageBox::Ok|QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    QWoUtils::removeDirectory(path);
    reload();
}

void QWoPlaybookManageDialog::onPlaybooksItemDbClicked(const QModelIndex &idx)
{
    QString path = itemToPath(idx);
    if(path.isEmpty()) {
        QKxMessageBox::information(this, tr("Playbook information"), tr("No item was selected to continue."));
        return;
    }
    done(QDialog::Accepted+1);
}

void QWoPlaybookManageDialog::reload()
{
    QString path = QWoSetting::playbooksPath();
    m_books = reload(path);
    QString cpath = QWoSetting::customPlaybooksPath();
    m_books += reload(cpath);
    QStringList names;
    for(int i = 0; i < m_books.length(); i++) {
        const BookData& bd = m_books.at(i);
        names.append(bd.name);
    }
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->playbooks->model());
    model->setStringList(names);
}

QList<QWoPlaybookManageDialog::BookData> QWoPlaybookManageDialog::reload(const QString &path)
{
    QList<BookData> books;
    if(path.isEmpty()) {
        return books;
    }
    QDir d(path);
    QStringList dirs = d.entryList(QDir::NoDotAndDotDot|QDir::Dirs);
    for(auto it = dirs.begin(); it != dirs.end(); it++) {
        QString name = *it;
        QString file = path + "/" + name + "/playbook.json";
        if(!QFile::exists(file)) {
            continue;
        }
        QFile f(file);
        if(!f.open(QFile::ReadOnly)) {
            continue;
        }
        QJsonDocument jsdoc = QJsonDocument::fromJson(f.readAll());
        if(!jsdoc.isObject()) {
            continue;
        }
        BookData book;
        book.name = jsdoc["name"].toString();
        book.description = jsdoc["description"].toString();
        book.path = path + "/" + name;
        books.append(book);
    }
    return books;
}

QString QWoPlaybookManageDialog::itemToPath(const QModelIndex &idx) const
{
    int irow = idx.row();
    if(irow < 0 || irow >= m_books.length()) {
        return QString();
    }
    const BookData& book = m_books.at(irow);
    return book.path;
}

QString QWoPlaybookManageDialog::itemToName(const QModelIndex &idx) const
{
    int irow = idx.row();
    if(irow < 0 || irow >= m_books.length()) {
        return QString();
    }
    const BookData& book = m_books.at(irow);
    return book.name;
}
