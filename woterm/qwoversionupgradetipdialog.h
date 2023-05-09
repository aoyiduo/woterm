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

#ifndef QWOVERSIONUPGRADETIPDIALOG_H
#define QWOVERSIONUPGRADETIPDIALOG_H

#include <QDialog>
#include <QPointer>

namespace Ui {
class QWoVersionUpgradeTipDialog;
}

class QWoVersionUpgradeTipDialog : public QDialog
{
    Q_OBJECT

public:
    ~QWoVersionUpgradeTipDialog();

    static void check(QWidget *parent, bool isAuto);

protected:
    explicit QWoVersionUpgradeTipDialog(bool isAuto, QWidget *parent = nullptr);

    Q_INVOKABLE void checkVersion();
    Q_INVOKABLE void getVersionDescription();

private slots:
    void onVersionCheck(int code, const QByteArray& body);
    void onVersionDescriptionGet(int code, const QByteArray& body);
    void onIgnoreButtonClicked();
    void on7DaysButtonClicked();
    void onTomorrowButtonClicked();
    void onUpgradeButtonClicked();

private:
    Q_INVOKABLE bool handleVersionCheck(int code, const QByteArray& body);
private:
    Ui::QWoVersionUpgradeTipDialog *ui;
    QPointer<QWidget> m_parent;
    bool m_isAuto;
    QString m_verBody;
};

#endif // QWOVERSIONUPGRADETIPDIALOG_H
