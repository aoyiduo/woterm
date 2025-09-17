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

#include "qkxutils.h"

#include "qkxkeytranslator.h"

#include <QMap>
#include <QDir>
#include <QObject>
#include <QFontDatabase>
#include <QDebug>
#include <QFontMetricsF>
#include <QFontInfo>
#include <QDataStream>
#include <QCoreApplication>

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

static QMap<QString, QString> layouts;
static QString gkeytabCustomPath;
void QKxUtils::setCustomKeytabPath(const QString &path)
{
    gkeytabCustomPath = path;
}

QStringList QKxUtils::availableKeytabs()
{
    if(layouts.isEmpty()) {
        QStringList paths(gkeytabCustomPath);
        QString path = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../private/keytabs");
        paths.prepend(path);
        QStringList filters;
        filters << "*.keytab";
        for(auto it = paths.begin(); it != paths.end(); it++) {
            QString path = *it;
            QDir dir;
            if(dir.cd(path)) {
                QList<QFileInfo> fis = dir.entryInfoList(filters, QDir::Files);
                for(int i = 0; i < fis.length(); i++) {
                    QFileInfo& fi = fis[i];
                    QString fileName = fi.baseName();
                    layouts.insert(fileName, fi.absoluteFilePath());
                }
            }
        }
    }
    return layouts.keys();
}

QString QKxUtils::keytabPath(const QString &name)
{
    if(layouts.isEmpty()) {
        availableKeytabs();
    }
    return layouts.value(name);
}

void QKxUtils::cleanupKeytabs()
{
    layouts.clear();
}

static QMap<QString, QString> schemas;
static QString gschemaCustomPath;
void QKxUtils::setCustomColorSchemaPath(const QString &path)
{
    if(gschemaCustomPath.contains(path)) {
        return;
    }
    gschemaCustomPath.append(path);
}

QStringList QKxUtils::availableColorSchemas()
{
    if(schemas.isEmpty()) {
        QStringList paths(gschemaCustomPath);
        QString path = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../private/themes");
        paths.prepend(path);
        QStringList filters;
        filters << "*.theme";
        for(auto it = paths.begin(); it != paths.end(); it++) {
            QString path = *it;
            QDir dir;
            if(dir.cd(path)) {
                QList<QFileInfo> fis = dir.entryInfoList(filters, QDir::Files);
                for(int i = 0; i < fis.length(); i++) {
                    QFileInfo& fi = fis[i];
                    QString fileName = fi.fileName();
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

struct FontCustomInfo {
    int fid;
    QStringList families;
};

#define FONT_FILE_DELETE      ("fontDels.txt")
static QList<QString> gsysFamilies;
static QString gfontCustomPath;
static QMap<QString,FontCustomInfo> gcustomFonts;
void QKxUtils::setCustomFontPath(const QString &path)
{
    gfontCustomPath = path;
}

QStringList QKxUtils::customFontFamilies()
{
    QDir d(gfontCustomPath);

    systemFontFamilies();

    QStringList fileDels = removeFontList();
    QStringList fs = d.entryList(QDir::Files|QDir::NoDotAndDotDot);
    for(auto it = fs.begin(); it != fs.end(); it++) {
        QString name = *it;
        QString fileName = gfontCustomPath + "/" + name;
        if(name == FONT_FILE_DELETE) {
            continue;
        }
        if(fileDels.contains(fileName)) {
            continue;
        }
        if(gcustomFonts.contains(fileName)) {
            continue;
        }
        int fid = QFontDatabase::addApplicationFont(fileName);
        if(fid < 0) {
            continue;
        }
        FontCustomInfo fci;
        fci.fid = fid;
        fci.families = QFontDatabase::applicationFontFamilies(fid);
        gcustomFonts.insert(fileName, fci);
    }
    QStringList customs;
    for(auto it = gcustomFonts.begin(); it != gcustomFonts.end(); it++) {
        FontCustomInfo& fci = *it;
        for(int i = 0; i < fci.families.length(); i++) {
            QString tmp = fci.families.at(i);
            if(customs.contains(tmp)) {
                continue;
            }
            if(gsysFamilies.contains(tmp)) {
                continue;
            }
            customs.append(tmp);
        }
    }
    return customs;
}


void QKxUtils::removeCustomFontFamily(const QString &name, QStringList& fileErrs)
{
    for(auto it = gcustomFonts.begin(); it != gcustomFonts.end(); ) {
        QString fileName = it.key();
        const FontCustomInfo& fci = it.value();
        if(fci.families.contains(name)) {
            if(!QFontDatabase::removeApplicationFont(fci.fid)) {
                fileErrs.append(fileName);
                it++;
                continue;
            }
            it = gcustomFonts.erase(it);
            if(!QFile::remove(fileName)) {
                fileErrs.append(fileName);
            }
        }else{
            it++;
        }
    }
    if(!fileErrs.isEmpty()){
        appentFontRemoveList(fileErrs);
    }
}

QStringList QKxUtils::systemFontFamilies()
{
    if(gsysFamilies.isEmpty()) {
        static QFontDatabase fdb;
        QStringList list = fdb.families(QFontDatabase::Any);
        for (int i = 0; i < list.size(); ++i) {
            // fix pitch = fix width.
            if(!fdb.isFixedPitch(list.at(i))) {
                continue;
            }
            gsysFamilies.append(list.at(i));
        }
        qDebug() << "font List" << gsysFamilies;
    }
    return gsysFamilies;
}

QStringList QKxUtils::availableFontFamilies()
{
    QStringList sysFamilies = systemFontFamilies();
    QStringList customs = customFontFamilies();
    customs.append(sysFamilies);
    return customs;
}

bool QKxUtils::isFixedPitch(const QFont &font) {
    const QFontInfo fi(font);
    qDebug() << fi.family() << fi.fixedPitch();
    return fi.fixedPitch();
}

QStringList QKxUtils::fontRemoveList()
{
    QString fileName = gfontCustomPath + "/" + FONT_FILE_DELETE;
    QStringList fileDels;
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)) {
        return fileDels;
    }
    QDataStream ds(&file);
    ds >> fileDels;
    return fileDels;
}

QStringList QKxUtils::removeFontList()
{
    QStringList fileDels = fontRemoveList();
    for(auto it = fileDels.begin(); it != fileDels.end(); ) {
        QString fileName = *it;
        if(!QFile::exists(fileName)) {
            it = fileDels.erase(it);
        }else if(QFile::remove(fileName)) {
            it = fileDels.erase(it);
        }else{
            it++;
        }
    }
    if(fileDels.isEmpty()) {
        QString fileName = gfontCustomPath + "/" + FONT_FILE_DELETE;
        QFile::remove(fileName);
    }
    return fileDels;
}

void QKxUtils::appentFontRemoveList(const QStringList &_dels)
{
    QStringList dels = _dels;
    QStringList fileDels = fontRemoveList();
    for(auto it = dels.begin(); it != dels.end(); ) {
        QString del = *it;
        if(fileDels.contains(del)) {
            it = dels.erase(it);
        }else{
            it++;
        }
    }
    fileDels.append(dels);
    QString fileName = gfontCustomPath + "/" + FONT_FILE_DELETE;
    QFile file(fileName);
    if(file.open(QFile::WriteOnly)) {
        QDataStream ds(&file);
        ds << fileDels;
    }
}

QStringList QKxUtils::suggestFamilies()
{
    QStringList suggest;
#if defined (Q_OS_WIN)
    suggest.append("Courier New");
    suggest.append("Courier");
    suggest.append("Terminal");
#elif defined(Q_OS_MAC)
    suggest.append("Courier New");
    suggest.append("Courier");
    suggest.append("SF Mono");
    suggest.append("Monaco");
    suggest.append("Menlo");
#else
    suggest.append("Courier 10 Pitch");
    suggest.append("Monospace");
#endif
    return suggest;
}

QStringList QKxUtils::familyStyles(const QString &family)
{
    static QFontDatabase fdb;
    QStringList styles = fdb.styles(family);
    if(styles.isEmpty()) {
        styles.append(""); // style may be is empty.
    }
    return styles;
}

int QKxUtils::suggestFontSize(const QString &family, int pt)
{
    static QFontDatabase fdb;
    QList<int> pts = fdb.pointSizes(family, QString());
    if(pts.isEmpty()) {
        return pt;
    }
    int pt_distance = 100;
    for(auto it = pts.begin(); it != pts.end(); it++) {
        int tmp = *it;
        if(tmp == pt) {
            return pt;
        }
        if(qAbs(tmp - pt) < qAbs(pt_distance)) {
            pt_distance = tmp - pt;
        }
    }
    return pt + pt_distance;
}
