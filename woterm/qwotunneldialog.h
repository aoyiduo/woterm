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

#ifndef QWOTUNNELDIALOG_H
#define QWOTUNNELDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoTunnelDialog;
}

class QWoTunnelModel;
class QWoTunnelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoTunnelDialog(QWidget *parent = nullptr);
    ~QWoTunnelDialog();

private slots:
    void onAddButtonClicked();
    void onModifyButtonClicked();
    void onRemoveButtonClicked();
    void onTunnelListModelReset();
    void onTunnelListItemClicked(const QModelIndex& idx);
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onSwitchButtonClicked();

    void onTunnelErrorArrived(const QString& err);
private:
    Q_INVOKABLE void appendLog(const QString& sessionName, const QString& msg);
    Q_INVOKABLE void resetState();
    virtual void showEvent(QShowEvent* ev);
    virtual void hideEvent(QHideEvent* ev);
private:
    Ui::QWoTunnelDialog *ui;
    QPointer<QWoTunnelModel> m_model;
};

#endif // QWOTUNNELDIALOG_H
