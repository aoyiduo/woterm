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

#include "qkxkeytranslatormodel.h"
#include "qkxutils.h"

#include <QSize>
#include <QFontMetrics>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

QKxKeyTranslatorModel::QKxKeyTranslatorModel(const QString &pathPrv, const QFont& font, QObject *parent)
    : QAbstractListModel(parent)
    , m_font(font)
    , m_hasModified(false)
{
    m_pathPrivate = QDir::cleanPath(pathPrv);
    m_opIcon = QIcon("../private/skins/black/operation.png");
    m_strIcon = QIcon("../private/skins/black/string.png");
}

QString QKxKeyTranslatorModel::name() const
{
    QFileInfo fi(m_filePath);
    return fi.fileName();
}

bool QKxKeyTranslatorModel::save(const QString& fileTemplate)
{
    if(m_filePath.startsWith(m_pathPrivate)) {
        return false;
    }
    if(!QKxKeyTranslator::save(m_filePath, fileTemplate, m_keys)) {
        return false;
    }
    m_hasModified = false;
    return true;
}

bool QKxKeyTranslatorModel::load(const QString &name)
{
    QString path = QKxUtils::keytabPath(name);
    m_keys.clear();
    beginResetModel();
    bool ok = QKxKeyTranslator::load(path, m_keys);
    mysort();
    endResetModel();
    m_filePath = path;
    m_hasModified = false;
    return ok;
}

bool QKxKeyTranslatorModel::hasModified()
{
    return m_hasModified;
}

QModelIndex QKxKeyTranslatorModel::modify(const QModelIndex& idx, QKxKeyTranslator::KeyInfo &ki)
{
    if(!idx.isValid()) {
        return idx;
    }
    int row = idx.row();
    beginResetModel();
    m_keys[row] = ki;
    mysort();
    int i = 0;
    for(i = 0; i < m_keys.length(); i++) {
        const QKxKeyTranslator::KeyInfo& hit = m_keys.at(i);
        if(hit == ki) {
            break;
        }
    }
    endResetModel();
    m_hasModified = true;
    return index(i);
}

QModelIndex QKxKeyTranslatorModel::add(QKxKeyTranslator::KeyInfo &ki)
{
    beginResetModel();
    m_keys.append(ki);
    mysort();
    int i = 0;
    for(i = 0; i < m_keys.length(); i++) {
        const QKxKeyTranslator::KeyInfo& hit = m_keys.at(i);
        if(hit == ki) {
            break;
        }
    }
    endResetModel();
    m_hasModified = true;
    return index(i);
}

void QKxKeyTranslatorModel::remove(const QModelIndex &idx)
{
    if(!idx.isValid()) {
        return;
    }
    int row = idx.row();
    beginResetModel();
    m_keys.removeAt(row);
    mysort();
    endResetModel();
    m_hasModified = true;
}

bool QKxKeyTranslatorModel::data(int idx, QKxKeyTranslator::KeyInfo &ki) const
{
    if(idx < 0 || idx >= m_keys.length()) {
        return false;
    }
    ki = m_keys.at(idx);
    return true;
}

QVariant QKxKeyTranslatorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Combine key");
        case 1:
            return tr("Action");
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

int QKxKeyTranslatorModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }


    return m_keys.length();
}

QVariant QKxKeyTranslatorModel::data(const QModelIndex &idx, int role) const
{
    int row = idx.row();
    int column = idx.column();
    if (!idx.isValid() || row < 0 || row >= m_keys.length()) {
        return QVariant();
    }

    const QKxKeyTranslator::KeyInfo& ki = m_keys.at(row);
    if(role == Qt::DisplayRole) {
        if(column == 0) {
            return ki.condition;
        }else if(column == 1) {
            auto all = QKxKeyTranslator::operationDescriptions();
            if(ki.out.isEmpty()) {
                return all.value(ki.op);
            }
            return ki.action;
        }
        return QVariant();
    }else if(role == Qt::DecorationRole) {
        if(column == 0) {
            return ki.out.isEmpty() ? m_opIcon : m_strIcon;
        }
        return QVariant();
    }
    if(role == Qt::SizeHintRole) {
        QFontMetrics fm(m_font);
        if(column == 0) {
            int w = fm.width(ki.condition) + 50;
            return QSize(w, 28);
        }
        if(column == 1) {
            int w = fm.width(ki.action) + 30;
            return QSize(w, 28);
        }
        return QVariant();
    }
    return QVariant();
}

int QKxKeyTranslatorModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

void QKxKeyTranslatorModel::mysort()
{
    std::sort(m_keys.begin(), m_keys.end(), [=](const QKxKeyTranslator::KeyInfo& v1, const QKxKeyTranslator::KeyInfo& v2){
        if(v1.out.isEmpty() && v2.out.isEmpty()) {
            return v1.op < v2.op;
        }else if(!v1.out.isEmpty() && !v2.out.isEmpty()) {
            int vkey1 = v1.key + (v1.out.isEmpty() ? 0 : 1 << 30);
            int vkey2 = v2.key + (v2.out.isEmpty() ? 0 : 1 << 30);
            QString key1 = QString::number(vkey1) + v1.out;
            QString key2 = QString::number(vkey2) + v2.out;
            return key1 < key2;
        }
        int vkey1 = v1.key + (v1.out.isEmpty() ? 0 : 1 << 30);
        int vkey2 = v2.key + (v2.out.isEmpty() ? 0 : 1 << 30);
        return vkey1 < vkey2;
    });
}
