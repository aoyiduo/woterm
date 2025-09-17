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

#ifndef QWOFONTLISTMODEL_H
#define QWOFONTLISTMODEL_H

#include <QStringListModel>

class QWoFontListModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit QWoFontListModel(QObject *parent = nullptr);
    virtual ~QWoFontListModel();
    Q_INVOKABLE void reload();
protected:
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QStringList m_fontsSystem, m_fontsImport;
};

#endif // QWOFONTLISTMODEL_H
