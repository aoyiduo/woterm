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

#ifndef QWODBSERVERMERGEMODEL_H
#define QWODBSERVERMERGEMODEL_H

#include "qwodbmergemodel.h"

class QWoDBServerMergeModel : public QWoDBMergeModel
{
    Q_OBJECT
public:
    explicit QWoDBServerMergeModel(QObject *parent = nullptr);
private:
    virtual void runApply();
    virtual QString toString(const QVariantMap& data) const;
};

#endif // QWODBSERVERMERGEMODEL_H
