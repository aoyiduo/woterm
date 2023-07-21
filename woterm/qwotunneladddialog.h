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

#ifndef QWOTUNNELADDDIALOG_H
#define QWOTUNNELADDDIALOG_H

#include "qwotunnelmodel.h"

#include <QDialog>

namespace Ui {
class QWoTunnelAddDialog;
}

class QWoTunnelAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoTunnelAddDialog(QWidget *parent = nullptr);
    ~QWoTunnelAddDialog();

    TunnelInfo result() const;
private slots:
    void onApplyButtonClicked();
    void onSessionsButtonClicked();
    void onTypeCurrentIndexChanged(int idx);
    void onHostLocalAssistClicked();
    void onHostRemoteAssistClicked();
protected:
    Ui::QWoTunnelAddDialog *ui;
    qint64 m_id;
};

#endif // QWOTUNNELADDDIALOG_H
