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
