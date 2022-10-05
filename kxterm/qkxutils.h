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

#ifndef QUTILS_H
#define QUTILS_H

#include "qvtedef.h"

#include <QColor>
#include <QFont>

class QKxKeyTranslator;

class QTERM_EXPORT QKxUtils
{
public:
    static QStringList availableKeyLayouts();
    static QKxKeyTranslator *keyboardLayout(const QString& name);
    static QStringList availableColorSchemas();
    static QString colorSchemaPath(const QString& name);
    static QStringList availableFontFamilies();
    static bool hasFamily(const QString& family);
    static QString suggestFamily();
    static QFont suggestFont();
    static bool fontCanUsed(const QFont& ft);
    static bool fontCanUsed(const QString& family);
    static bool generateSpecialCharactorWidth();
    static int specialCharactorCount(wchar_t c);

};

#endif // QUTILS_H
