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
    static void setCustomFontPath(const QString& path);
    static QStringList customFontFamilies();
    static void removeCustomFontFamily(const QString& name, QStringList& fileErrs);
    static QStringList systemFontFamilies();
    static QStringList availableFontFamilies();
    static QStringList suggestFamilies();
    static QStringList familyStyles(const QString& family);
    static int suggestFontSize(const QString& family, int pt);
    static bool isFixedPitch(const QFont &font);
private:
    static QStringList fontRemoveList();
    static QStringList removeFontList();
    static void appentFontRemoveList(const QStringList& dels);
};

#endif // QUTILS_H
