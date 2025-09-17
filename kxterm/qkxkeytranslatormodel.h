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

#ifndef QKXKEYTRANSLATORMODEL_H
#define QKXKEYTRANSLATORMODEL_H

#include "qvtedef.h"
#include "qkxkeytranslator.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QFont>
#include <QIcon>

class QTERM_EXPORT QKxKeyTranslatorModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit QKxKeyTranslatorModel(const QString& pathPrv, const QFont& font, QObject *parent = nullptr);

    QString name() const;
    bool save(const QString& fileTemplate);
    bool load(const QString& name = DEFAULT_KEY_TRANSLATOR);
    int count() {
        return m_keys.length();
    };

    bool hasModified();
    QModelIndex modify(const QModelIndex& idx, QKxKeyTranslator::KeyInfo& ki);
    QModelIndex add(QKxKeyTranslator::KeyInfo& ki);
    void remove(const QModelIndex& idx);
    bool data(int idx, QKxKeyTranslator::KeyInfo& ki) const;
protected:
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int columnCount(const QModelIndex &parent) const override;
private:
    void mysort();
private:
    QList<QKxKeyTranslator::KeyInfo> m_keys;
    QString m_filePath;
    QString m_pathPrivate;
    QFont m_font;
    QIcon m_opIcon, m_strIcon;
    bool m_hasModified;
};

#endif // QKXKEYTRANSLATORMODEL_H
