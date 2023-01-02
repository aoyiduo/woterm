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

#ifndef QWOSYSTEMOPTIONDIALOG_H
#define QWOSYSTEMOPTIONDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class QWoSystemOptionDialog;
}

class QWoSystemOptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSystemOptionDialog(QWidget *parent = nullptr);
    ~QWoSystemOptionDialog();
    QString language() const;
private slots:
    void onButtonSaveClicked();
private:
    Ui::QWoSystemOptionDialog *ui;
    QMap<QString, QString> m_langs;
    QString m_lang;
    QString m_result;
};

#endif // QWOSYSTEMOPTIONDIALOG_H
