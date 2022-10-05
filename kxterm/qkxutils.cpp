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

#include "qkxutils.h"

#include "qkxkeytranslator.h"

#include <QMap>
#include <QDir>
#include <QObject>
#include <QFontDatabase>
#include <QDebug>
#include <QFontMetricsF>

static QColor xterm_color256[] = {
    QColor("#000000"),
    QColor("#800000"),
    QColor("#008000"),
    QColor("#808000"),
    QColor("#000080"),
    QColor("#800080"),
    QColor("#008080"),
    QColor("#c0c0c0"),
    QColor("#808080"),
    QColor("#ff0000"),
    QColor("#00ff00"),
    QColor("#ffff00"),
    QColor("#0000ff"),
    QColor("#ff00ff"),
    QColor("#00ffff"),
    QColor("#ffffff"),
    QColor("#000000"),
    QColor("#00005f"),
    QColor("#000087"),
    QColor("#0000af"),
    QColor("#0000d7"),
    QColor("#0000ff"),
    QColor("#005f00"),
    QColor("#005f5f"),
    QColor("#005f87"),
    QColor("#005faf"),
    QColor("#005fd7"),
    QColor("#005fff"),
    QColor("#008700"),
    QColor("#00875f"),
    QColor("#008787"),
    QColor("#0087af"),
    QColor("#0087d7"),
    QColor("#0087ff"),
    QColor("#00af00"),
    QColor("#00af5f"),
    QColor("#00af87"),
    QColor("#00afaf"),
    QColor("#00afd7"),
    QColor("#00afff"),
    QColor("#00d700"),
    QColor("#00d75f"),
    QColor("#00d787"),
    QColor("#00d7af"),
    QColor("#00d7d7"),
    QColor("#00d7ff"),
    QColor("#00ff00"),
    QColor("#00ff5f"),
    QColor("#00ff87"),
    QColor("#00ffaf"),
    QColor("#00ffd7"),
    QColor("#00ffff"),
    QColor("#5f0000"),
    QColor("#5f005f"),
    QColor("#5f0087"),
    QColor("#5f00af"),
    QColor("#5f00d7"),
    QColor("#5f00ff"),
    QColor("#5f5f00"),
    QColor("#5f5f5f"),
    QColor("#5f5f87"),
    QColor("#5f5faf"),
    QColor("#5f5fd7"),
    QColor("#5f5fff"),
    QColor("#5f8700"),
    QColor("#5f875f"),
    QColor("#5f8787"),
    QColor("#5f87af"),
    QColor("#5f87d7"),
    QColor("#5f87ff"),
    QColor("#5faf00"),
    QColor("#5faf5f"),
    QColor("#5faf87"),
    QColor("#5fafaf"),
    QColor("#5fafd7"),
    QColor("#5fafff"),
    QColor("#5fd700"),
    QColor("#5fd75f"),
    QColor("#5fd787"),
    QColor("#5fd7af"),
    QColor("#5fd7d7"),
    QColor("#5fd7ff"),
    QColor("#5fff00"),
    QColor("#5fff5f"),
    QColor("#5fff87"),
    QColor("#5fffaf"),
    QColor("#5fffd7"),
    QColor("#5fffff"),
    QColor("#870000"),
    QColor("#87005f"),
    QColor("#870087"),
    QColor("#8700af"),
    QColor("#8700d7"),
    QColor("#8700ff"),
    QColor("#875f00"),
    QColor("#875f5f"),
    QColor("#875f87"),
    QColor("#875faf"),
    QColor("#875fd7"),
    QColor("#875fff"),
    QColor("#878700"),
    QColor("#87875f"),
    QColor("#878787"),
    QColor("#8787af"),
    QColor("#8787d7"),
    QColor("#8787ff"),
    QColor("#87af00"),
    QColor("#87af5f"),
    QColor("#87af87"),
    QColor("#87afaf"),
    QColor("#87afd7"),
    QColor("#87afff"),
    QColor("#87d700"),
    QColor("#87d75f"),
    QColor("#87d787"),
    QColor("#87d7af"),
    QColor("#87d7d7"),
    QColor("#87d7ff"),
    QColor("#87ff00"),
    QColor("#87ff5f"),
    QColor("#87ff87"),
    QColor("#87ffaf"),
    QColor("#87ffd7"),
    QColor("#87ffff"),
    QColor("#af0000"),
    QColor("#af005f"),
    QColor("#af0087"),
    QColor("#af00af"),
    QColor("#af00d7"),
    QColor("#af00ff"),
    QColor("#af5f00"),
    QColor("#af5f5f"),
    QColor("#af5f87"),
    QColor("#af5faf"),
    QColor("#af5fd7"),
    QColor("#af5fff"),
    QColor("#af8700"),
    QColor("#af875f"),
    QColor("#af8787"),
    QColor("#af87af"),
    QColor("#af87d7"),
    QColor("#af87ff"),
    QColor("#afaf00"),
    QColor("#afaf5f"),
    QColor("#afaf87"),
    QColor("#afafaf"),
    QColor("#afafd7"),
    QColor("#afafff"),
    QColor("#afd700"),
    QColor("#afd75f"),
    QColor("#afd787"),
    QColor("#afd7af"),
    QColor("#afd7d7"),
    QColor("#afd7ff"),
    QColor("#afff00"),
    QColor("#afff5f"),
    QColor("#afff87"),
    QColor("#afffaf"),
    QColor("#afffd7"),
    QColor("#afffff"),
    QColor("#d70000"),
    QColor("#d7005f"),
    QColor("#d70087"),
    QColor("#d700af"),
    QColor("#d700d7"),
    QColor("#d700ff"),
    QColor("#d75f00"),
    QColor("#d75f5f"),
    QColor("#d75f87"),
    QColor("#d75faf"),
    QColor("#d75fd7"),
    QColor("#d75fff"),
    QColor("#d78700"),
    QColor("#d7875f"),
    QColor("#d78787"),
    QColor("#d787af"),
    QColor("#d787d7"),
    QColor("#d787ff"),
    QColor("#d7af00"),
    QColor("#d7af5f"),
    QColor("#d7af87"),
    QColor("#d7afaf"),
    QColor("#d7afd7"),
    QColor("#d7afff"),
    QColor("#d7d700"),
    QColor("#d7d75f"),
    QColor("#d7d787"),
    QColor("#d7d7af"),
    QColor("#d7d7d7"),
    QColor("#d7d7ff"),
    QColor("#d7ff00"),
    QColor("#d7ff5f"),
    QColor("#d7ff87"),
    QColor("#d7ffaf"),
    QColor("#d7ffd7"),
    QColor("#d7ffff"),
    QColor("#ff0000"),
    QColor("#ff005f"),
    QColor("#ff0087"),
    QColor("#ff00af"),
    QColor("#ff00d7"),
    QColor("#ff00ff"),
    QColor("#ff5f00"),
    QColor("#ff5f5f"),
    QColor("#ff5f87"),
    QColor("#ff5faf"),
    QColor("#ff5fd7"),
    QColor("#ff5fff"),
    QColor("#ff8700"),
    QColor("#ff875f"),
    QColor("#ff8787"),
    QColor("#ff87af"),
    QColor("#ff87d7"),
    QColor("#ff87ff"),
    QColor("#ffaf00"),
    QColor("#ffaf5f"),
    QColor("#ffaf87"),
    QColor("#ffafaf"),
    QColor("#ffafd7"),
    QColor("#ffafff"),
    QColor("#ffd700"),
    QColor("#ffd75f"),
    QColor("#ffd787"),
    QColor("#ffd7af"),
    QColor("#ffd7d7"),
    QColor("#ffd7ff"),
    QColor("#ffff00"),
    QColor("#ffff5f"),
    QColor("#ffff87"),
    QColor("#ffffaf"),
    QColor("#ffffd7"),
    QColor("#ffffff"),
    QColor("#080808"),
    QColor("#121212"),
    QColor("#1c1c1c"),
    QColor("#262626"),
    QColor("#303030"),
    QColor("#3a3a3a"),
    QColor("#444444"),
    QColor("#4e4e4e"),
    QColor("#585858"),
    QColor("#606060"),
    QColor("#666666"),
    QColor("#767676"),
    QColor("#808080"),
    QColor("#8a8a8a"),
    QColor("#949494"),
    QColor("#9e9e9e"),
    QColor("#a8a8a8"),
    QColor("#b2b2b2"),
    QColor("#bcbcbc"),
    QColor("#c6c6c6"),
    QColor("#d0d0d0"),
    QColor("#dadada"),
    QColor("#e4e4e4"),
    QColor("#eeeeee")
};


QVector<QColor> initDefault() {
    QVector<QColor> vcs;
    int cnt = sizeof(xterm_color256) / sizeof(QColor);
    vcs.reserve(256);
    for(int i = 0; i < cnt; i++) {
        vcs.append(xterm_color256[i]);
    }
    return vcs;
}

static QMap<QString, QKxKeyTranslator*> layouts;
QStringList QKxUtils::availableKeyLayouts()
{
    if(layouts.isEmpty()) {
        QDir dir;
        if(dir.cd(":/kxterm/kblayout")) {
            QList<QFileInfo> fis = dir.entryInfoList(QDir::Files);
            for(int i = 0; i < fis.length(); i++) {
                QFileInfo& fi = fis[i];
                QString fileName = fi.baseName();
                QKxKeyTranslator *keyLayout = new QKxKeyTranslator();
                if(keyLayout->load(fi.absoluteFilePath())){
                    keyLayout->setName(fileName);
                    layouts.insert(fileName, keyLayout);
                }
            }
        }
    }
    return layouts.keys();
}

QKxKeyTranslator *QKxUtils::keyboardLayout(const QString &name)
{
    if(layouts.isEmpty()) {
        availableKeyLayouts();
    }
    if(layouts.isEmpty()) {
        return nullptr;
    }
    QKxKeyTranslator *key = layouts.value(name);
    if(key != nullptr) {
        return key;
    }
    return layouts.first();
}

static QMap<QString, QString> schemas;
QStringList QKxUtils::availableColorSchemas()
{
    if(schemas.isEmpty()) {
        QDir dir;
        if(dir.cd(":/kxterm/theme")) {
            QList<QFileInfo> fis = dir.entryInfoList(QDir::Files);
            for(int i = 0; i < fis.length(); i++) {
                QFileInfo& fi = fis[i];
                QString fileName = fi.fileName();
                if(fileName.endsWith(".theme")){
                    schemas.insert(fi.baseName(), fi.absoluteFilePath());
                }
            }
        }
    }
    return schemas.keys();
}

QString QKxUtils::colorSchemaPath(const QString &name)
{
    if(schemas.isEmpty()) {
        availableColorSchemas();
    }
    return schemas.value(name, "");
}


static QList<QString> families;
QStringList QKxUtils::availableFontFamilies()
{
    if(families.isEmpty()) {
        QFontDatabase::addApplicationFont(":/kxterm/font/CourierNew.ttf");
        QFontDatabase::addApplicationFont(":/kxterm/font/CourierNewBold.ttf");
        QFontDatabase fdb;
        QStringList list = fdb.families(QFontDatabase::Any);
        for (int i = 0; i < list.size(); ++i) {
            // fix pitch = fix width.
            if(!fdb.isFixedPitch(list.at(i))) {
                continue;
            }
            // check bold and normal equal.
           // if(!fontCanUsed(list.at(i))) {
           //     continue;
           // }
            families.append(list.at(i));
        }
        qDebug() << "font List" << families;
    }
    return families;
}

bool QKxUtils::hasFamily(const QString &family)
{
    if(families.isEmpty()) {
        availableFontFamilies();
    }
    return families.contains(family);
}

QString QKxUtils::suggestFamily()
{
    if(families.isEmpty()) {
        availableFontFamilies();
    }
    if(families.contains(DEFAULT_FONT_FAMILY)) {
        return DEFAULT_FONT_FAMILY;
    }
    return families.first();
}

QFont QKxUtils::suggestFont()
{
    QFont font;
    QString suggest = QKxUtils::suggestFamily();
    font.setFamily(suggest);
    font.setPointSize(DEFAULT_FONT_SIZE);
    font.setWeight(QFont::Normal);
    font.setStyleHint(QFont::Monospace);
    font.setStyleStrategy(QFont::StyleStrategy(QFont::ForceIntegerMetrics|QFont::PreferAntialias));
    font.setFixedPitch(true);
    font.setKerning(false);
    font.setStyleName(QString());
    return font;
}

// don't support italic font.
bool QKxUtils::fontCanUsed(const QFont &ft)
{
    const QString& hit("OoDdWMwm.12");
    QFontMetricsF fm(ft);
    QSizeF sz = fm.size(Qt::TextSingleLine, hit);
    QFont ft2 = ft;
    ft2.setBold(true);   
    QFontMetricsF fm2(ft2);
    QSizeF sz2 = fm2.size(Qt::TextSingleLine, hit);
    return sz.width() == sz2.width();
}

bool QKxUtils::fontCanUsed(const QString &family)
{
    QFont ft(family);
    ft.setFamily(family);
    ft.setPointSize(DEFAULT_FONT_SIZE);
    ft.setWeight(QFont::Normal);
    ft.setStyleHint(QFont::TypeWriter);
    ft.setStyleStrategy(QFont::ForceIntegerMetrics);
    ft.setFixedPitch(true);
    return fontCanUsed(ft);
}

static QHash<int,int> m_box;
int QKxUtils::specialCharactorCount(wchar_t c)
{
    if(m_box.isEmpty()) {
        QFont font = QKxUtils::suggestFont();
        QFontMetrics fm(font);

        //https://en.wikipedia.org/wiki/Box-drawing_character
        // box draw.
        int fw = fm.width("W");
        // box draw
        for(int i = 0x2500; i <= 0x257F; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
            //qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
        }

        //block draw
        for(int i = 0x2580; i <= 0x259F; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
            //qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
        }

        // special symbols
        for(int i = 0x1FB00; i <= 0x1FBFF; i++) {
            int w = fm.width(i);
            m_box.insert(i, qRound(float(w) / fw));
        }
    }
    return m_box.value(c, 0);
}

bool QKxUtils::generateSpecialCharactorWidth()
{
    QFont font;
    QString suggest = QKxUtils::suggestFamily();
    font.setFamily(suggest);
    font.setPixelSize(DEFAULT_FONT_SIZE);
    font.setWeight(QFont::Normal);
    font.setStyleHint(QFont::Monospace);
    font.setStyleStrategy(QFont::StyleStrategy(QFont::ForceIntegerMetrics|QFont::PreferAntialias));
    font.setFixedPitch(true);
    font.setKerning(false);
    font.setStyleName(QString());
    QFontMetrics fm(font);

    //https://en.wikipedia.org/wiki/Box-drawing_character
    // box draw.
    int fw = fm.width("W");
    // box draw
    for(int i = 0x2500; i <= 0x257F; i++) {
        int w = fm.width(i);
        qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
    }

    //block draw
    for(int i = 0x2580; i <= 0x259F; i++) {
        int w = fm.width(i);
        qDebug() << QString("{ 0x%1, %2, %3, %4 },").arg(i, 0, 16).arg(qRound(float(w) / fw)).arg(fw).arg(w);
    }
    //if(c >= 0x2500 && c <= 0x2573) {
    //    return 1;
    //}
   // if(c >= 0x2574 && c <= 0x257F) {
    //    return 2;
    //}
    // block draw
    //if(c >= 0x2580 && c <= 0x259F) {
    //    return 2;
    //}
    return true;
}
