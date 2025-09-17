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

#ifndef QWOSYSTEMOPTIONDIALOG_H
#define QWOSYSTEMOPTIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QList>

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
    QString skin() const;
private slots:
    void onButtonSaveClicked();
    void onSkinCurrentIndexChanged(const QString& name);
private:
    Ui::QWoSystemOptionDialog *ui;
    QMap<QString, QString> m_langs;
    QMap<QString, QString> m_skins;
    QString m_lang;
    QString m_result;
};

#endif // QWOSYSTEMOPTIONDIALOG_H
