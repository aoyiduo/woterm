/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QWODBIDENTITYMERGEMODEL_H
#define QWODBIDENTITYMERGEMODEL_H

#include "qwodbmergemodel.h"

class QWoDBIdentityMergeModel : public QWoDBMergeModel
{
    Q_OBJECT
public:
    explicit QWoDBIdentityMergeModel(QObject *parent = nullptr);

private:
    virtual void runApply();
    virtual QString toString(const QVariantMap& data) const;

};

#endif // QWODBIDENTITYMERGEMODEL_H
