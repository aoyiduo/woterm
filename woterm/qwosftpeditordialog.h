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

#ifndef QWOSFTPEDITORDIALOG_H
#define QWOSFTPEDITORDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class QWoSftpEditorDialog;
}

class QWoSftpEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoSftpEditorDialog(const QStringList& watchs, QWidget *parent = nullptr);
    ~QWoSftpEditorDialog();

signals:
    void ftpSettingArrived();
    void clearAll();
    void stopArrived(const QString& fileName);
private slots:
    void onClearAllButtonClicked();
    void onStopWatchButtonClicked();
private:
    Ui::QWoSftpEditorDialog *ui;
    QMap<QString, QString> m_watchs;
};

#endif // QWOSFTPEDITORDIALOG_H
