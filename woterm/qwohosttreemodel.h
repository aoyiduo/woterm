/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWOHOSTTREEMODEL_H
#define QWOHOSTTREEMODEL_H

#include <QStandardItemModel>

class QWoHostTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit QWoHostTreeModel(QObject* parent = nullptr);
    ~QWoHostTreeModel();
    static QWoHostTreeModel *instance();

    Q_INVOKABLE void refreshList();
private slots:
    void onDataReset();
private:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    QIcon m_sshIcon;
    QIcon m_sftpIcon;
    QIcon m_telnetIcon;
    QIcon m_rloginIcon;
    QIcon m_mstscIcon;
    QIcon m_vncIcon;
    QIcon m_serialIcon;
    QIcon m_folderOpenIcon;
    QIcon m_folderCloseIcon;
};

#endif // QWOHOSTTREEMODEL_H
