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

#ifndef QWODBMIGRATEDIALOG_H
#define QWODBMIGRATEDIALOG_H


#include "qwosshconf.h"

#include <QMap>
#include <QList>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>
#include <QDialog>

namespace Ui {
class QWoDBMigrateDialog;
}

class QWoDBMigrateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QWoDBMigrateDialog(QWidget *parent = nullptr);
    ~QWoDBMigrateDialog();

private slots:
    void onBrowserButtonClicked();
    void onImportSessionButtonClicked();
    void onSearchSessionPathButtonClicked();
    void onTypsCurrentIndexChanged(int idx);

private:
    void handleXShellSessionImport(const QString& path, QMap<QString, QList<HostInfo>>& all);
    void handleSecureCRTSessionImport(const QString& path, QMap<QString, QList<HostInfo>>& all);
    void handleSecureCRTSessionXMLImport(const QString& path, QMap<QString, QList<HostInfo>>& all);
    void handleSecureCRTSessionXMLImport(const QDomNode& node, QMap<QString, QList<HostInfo>>& all);
private:
    QDomNode findPath(const QDomNode& node, const QString& path);
    QString hostName(const QDomElement& node, const QString& vdef=QString());
    QString userName(const QDomElement& node, const QString& vdef=QString());
    QString protocol(const QDomElement& node, const QString& vdef=QString());
    QString port(const QDomElement& node, const QString& vdef=QString());
    QList<QDomElement> findChilds(const QDomElement& node,const QString& name, const QMap<QString, QString> &properties);
private:
    Ui::QWoDBMigrateDialog *ui;
};

#endif // QWODBMIGRATEDIALOG_H
