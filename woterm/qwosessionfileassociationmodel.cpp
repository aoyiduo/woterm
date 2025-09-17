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

#include "qwosessionfileassociationmodel.h"

#include "qwosetting.h"

#include <algorithm>

#include <QDateTime>
#include <QDataStream>
#include <QCoreApplication>
#include <QFileInfo>

#define FILE_ASSOCIATION_VERSION        ("v1.0")

QWoSessionFileAssociationModel::QWoSessionFileAssociationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    init();
}

QWoSessionFileAssociationModel *QWoSessionFileAssociationModel::instance()
{
    static QPointer<QWoSessionFileAssociationModel> gFileAssociation;
    if(gFileAssociation == nullptr) {
        gFileAssociation = new QWoSessionFileAssociationModel(QCoreApplication::instance());
    }
    return gFileAssociation;
}

bool QWoSessionFileAssociationModel::autoAddDefaultEditor() const
{
    return QWoSetting::value("sftp/autoAddDefaultEditor", false).toBool();
}

void QWoSessionFileAssociationModel::setAutoAddDefaultEditor(bool on)
{
    QWoSetting::setValue("sftp/autoAddDefaultEditor", on);
    if(on) {
        if(tryToAddDefaultEditor()) {
            beginResetModel();
            sort();
            save();
            endResetModel();
        }
     }
}

bool QWoSessionFileAssociationModel::findEditor(const QString &file, FileAssociation *pFileAssoc)
{
    int pos = file.lastIndexOf(".");
    if(pos <= 0) {
        return false;
    }
    QString suffix = file.mid(pos);
    for(auto it = m_all.begin(); it != m_all.end(); it++){
        const FileAssociation& fa = *it;
        QString fileTyps = fa.type;
        fileTyps = fileTyps.replace(" ", "");
        QStringList typs = fileTyps.split(',');
        if(typs.contains(suffix) || typs.contains("*")) {
            QFileInfo fi(fa.application);
            if(!fi.isExecutable()) {
                continue;
            }
            if(pFileAssoc) {
                *pFileAssoc = fa;
            }
            return true;
        }
    }
    return false;
}

void QWoSessionFileAssociationModel::add(const QString &typs, const QString& app, const QString& parameter, int priority)
{
    beginResetModel();
    auto it = std::find_if(m_all.begin(), m_all.end(), [=](const FileAssociation& fa){
        return typs == fa.type && fa.application == app;
    });
    if(it != m_all.end()) {
        m_all.erase(it);
    }
    FileAssociation fa;
    fa.type = typs;
    fa.application = app;
    fa.parameter = parameter;
    fa.priority = priority;
    m_all.append(fa);
    sort();
    endResetModel();
    save();
}

void QWoSessionFileAssociationModel::update(int idx, const QString &typs, const QString &app, const QString &parameter, int priorty)
{
    if(idx < 0 || idx >= m_all.length()) {
        return;
    }
    beginResetModel();
    FileAssociation fa;
    fa.type = typs;
    fa.application = app;
    fa.parameter = parameter;
    fa.priority = priorty;
    m_all[idx] = fa;
    sort();
    endResetModel();
    save();
}

void QWoSessionFileAssociationModel::remove(int idx)
{
    beginResetModel();
    m_all.removeAt(idx);    
    endResetModel();
    save();
}

void QWoSessionFileAssociationModel::clear()
{
    beginResetModel();
    m_all.clear();
    endResetModel();
    save();
}

int QWoSessionFileAssociationModel::rowCount(const QModelIndex &parent) const
{
    return m_all.length();
}

int QWoSessionFileAssociationModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant QWoSessionFileAssociationModel::data(const QModelIndex &idx, int role) const
{
    int row = idx.row();
    if(row >= m_all.length() || row < 0) {
        return QVariant();
    }
    const FileAssociation& fa = m_all.at(row);

    if(role == ROLE_INDEX) {
        return QVariant::fromValue<FileAssociation>(fa);
    }

    if(role == Qt::DisplayRole) {
        if(idx.column() == 0) {
            return fa.priority;
        }else if(idx.column() == 1) {
            return fa.type;
        }else if(idx.column() == 2) {
            return fa.parameter;
        }else if(idx.column() == 3) {
            return fa.application;
        }
    }
    return QVariant();
}

QVariant QWoSessionFileAssociationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == 0) {
                return tr("Priority");
            }else if(section == 1) {
                return tr("File types");
            }else if(section == 2) {
                return tr("Parameter");
            }else if(section == 3){
                return tr("Application");
            }
        }
    }
    return QVariant();
}

void QWoSessionFileAssociationModel::save()
{
    QByteArray buf;
    QDataStream ds(&buf, QIODevice::WriteOnly);
    ds << QString(FILE_ASSOCIATION_VERSION) << int(m_all.size());
    for(auto it = m_all.begin(); it != m_all.end(); it++) {
        const FileAssociation& fa = *it;
        ds << fa.type << fa.application << fa.parameter << fa.priority;
    }
    QWoSetting::setValue("sftp/fileAssociation", buf);
}

void QWoSessionFileAssociationModel::init()
{
    QByteArray buf = QWoSetting::value("sftp/fileAssociation").toByteArray();
    QDataStream ds(buf);
    QString version;
    int cnt;
    ds >> version >> cnt;
    if(version != FILE_ASSOCIATION_VERSION) {
        return;
    }
    beginResetModel();
    for(int i = 0; i < cnt; i++) {
        FileAssociation fa;
        ds >> fa.type >> fa.application >> fa.parameter >> fa.priority;
        m_all.append(fa);
    }

    tryToAddDefaultEditor();
    sort();
    endResetModel();
}

void QWoSessionFileAssociationModel::sort()
{
    std::sort(m_all.begin(), m_all.end(), [](const FileAssociation& v1, const FileAssociation& v2){
        return v1.priority < v2.priority;
    });
}

bool QWoSessionFileAssociationModel::tryToAddDefaultEditor()
{
    bool add = autoAddDefaultEditor();
    bool hasAdd = false;
    if(add){
        FileAssociation fa;
        fa.application = "/usr/bin/gedit";
        fa.parameter = "\"{file}\"";
        fa.type = "*";
        fa.priority = 65500;

        auto it = std::find_if(m_all.begin(), m_all.end(), [=](const FileAssociation& v){
            return v.type == fa.type && v.application == fa.application;
        });
        if(it == m_all.end()) {
            m_all.append(fa);
            hasAdd = true;
        }
    }
    if(add) {
        FileAssociation fa;
        fa.application = "c:\\windows\\notepad.exe";
        fa.parameter = "\"{file}\"";
        fa.type = "*";
        fa.priority = 65510;

        auto it = std::find_if(m_all.begin(), m_all.end(), [=](const FileAssociation& v){
            return v.type == fa.type && v.application == fa.application;
        });
        if(it == m_all.end()) {
            m_all.append(fa);
            hasAdd = true;
        }
    }
    if(add) {
        FileAssociation fa;
        fa.application = "/usr/bin/open";
        fa.parameter = "-e \"{file}\"";
        fa.type = "*";
        fa.priority = 65520;

        auto it = std::find_if(m_all.begin(), m_all.end(), [=](const FileAssociation& v){
            return v.type == fa.type && v.application == fa.application;
        });
        if(it == m_all.end()) {
            m_all.append(fa);
            hasAdd = true;
        }
    }
    return hasAdd;
}
