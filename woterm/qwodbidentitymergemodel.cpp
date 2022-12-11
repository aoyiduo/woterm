/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the GPLv3[GNU GENERAL PUBLIC LICENSE v3]
* more information follow the website: https://www.gnu.org/licenses/gpl-3.0.en.html
*
*******************************************************************************************/

#include "qwodbidentitymergemodel.h"

QWoDBIdentityMergeModel::QWoDBIdentityMergeModel(QObject *parent)
    : QWoDBMergeModel(parent)
{

}

QString QWoDBIdentityMergeModel::toString(const QVariantMap &dm) const
{
    QStringList names = {tr("name"),tr("prvKey")};
    QStringList lines;
    for(auto it = dm.begin(); it != dm.end(); it++) {
        QString key = it.key();
        QString txt = it.value().toString();
        if(key == "ct"
                || key == "delFlag"
                || key == "dt"
                || key == "id"
                || key == "syncFlag") {
            continue;
        }
        if(txt.isEmpty()) {
            continue;
        }
        QString line = tr(key.toUtf8()) + ":" + txt;
        lines.append(line);
    }
    QString out = lines.join("\r\n");
    return out;
}
