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

#ifndef QWOPLAYBOOKMANAGEDIALOG_H
#define QWOPLAYBOOKMANAGEDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QMap>
#include <QList>

namespace Ui {
class QWoPlaybookManageDialog;
}

class QWoPlaybookManageDialog : public QDialog
{
    Q_OBJECT

    struct BookData {
        QString name;
        QString path;
        QString description;
    };
public:
    explicit QWoPlaybookManageDialog(QWidget *parent = nullptr);
    ~QWoPlaybookManageDialog();

    QString name() const;
    QString path() const;

private slots:
    void onApplyButtonClicked();
    void onBrowserButtonClicked();
    void onConfigureButtonClicked();
    void onCopyButtonClicked();
    void onRefreshButtonClicked();
    void onRemoveButtonClicked();
    void onPlaybooksItemDbClicked(const QModelIndex& idx);
private:
    Q_INVOKABLE void reload();
    QList<BookData> reload(const QString& path);
    QString itemToPath(const QModelIndex& idx) const;
    QString itemToName(const QModelIndex& idx) const;
private:
    Ui::QWoPlaybookManageDialog *ui;    
    QList<BookData> m_books;
};

#endif // QWOPLAYBOOKMANAGEDIALOG_H
