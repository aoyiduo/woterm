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

#include "qwotheme.h"

#include <private/qabstractfileengine_p.h>
#include <private/qfsfileengine_p.h>
#include "private/qfilesystemengine_p.h"

#include "qwosetting.h"
#include "qwoapplication.h"

#include <QStyleFactory>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>


Q_GLOBAL_STATIC(QWoTheme, gWoTheme)


class FileEngineHandler : public QAbstractFileEngineHandler
{
private:
    QPointer<QWoTheme> m_theme;
public:
    FileEngineHandler(QWoTheme *theme)
        : QAbstractFileEngineHandler()
        , m_theme(theme){
    }

    virtual ~FileEngineHandler() {

    }

    virtual QAbstractFileEngine *create(const QString &file) const {
        return m_theme->create(file);
    }
};

QWoTheme::QWoTheme(QObject *parent)
    : QObject(parent)
{
    m_handler = new FileEngineHandler(this);
    QStringList styles = QStyleFactory::keys();
    qDebug() << "embeded style list: " << styles;
    QApplication::setStyle("fusion");
    reload();
    m_skinUniqueName = QWoSetting::value("skin/current", "black").toString();
    if(!loadSkinByUniqueName(m_skinUniqueName)) {
        loadSkinByUniqueName(m_skins.firstKey());
    }
}

QWoTheme::~QWoTheme()
{
    delete m_handler;
}

QWoTheme *QWoTheme::instance()
{
    return gWoTheme;
}

QString QWoTheme::skinUniqueName() const
{
    // do not read directly from configure, because maybe it would not exit or block any more when changed the skin.
    // return QWoSetting::value("skin/current", "black").toString();
    //
    return m_skinUniqueName;
}

void QWoTheme::setSkinUniqueName(const QString &name)
{
    QWoSetting::setValue("skin/current", name);
}

QString QWoTheme::skinPathByUniqueName(const QString &name)
{
    if(!m_skins.contains(name)) {
        return QString();
    }
    SkinData sd = m_skins.value(name);
    return sd.path;
}

QMap<QString, QString> QWoTheme::skinFriendNames()
{
    QMap<QString, QString> names;
    QString type = QWoSetting::languageTypeAsBCP47Field();
    for(auto it = m_skins.begin(); it != m_skins.end(); it++) {
        const SkinData& sd = *it;
        names.insert(it.key(), sd.names.value(type));
    }
    return names;
}

void QWoTheme::addCustomSkinPaths(const QStringList &paths)
{
    m_customSkinPaths = paths;
}

QIcon QWoTheme::icon(const QString &fileName)
{
    return QIcon();
}

QAbstractFileEngine *QWoTheme::create(const QString &file) const
{
    if(file.isEmpty()) {
        return nullptr;
    }
    if(!file.contains("/private/skins/")) {
        return nullptr;
    }
    if(file.endsWith("skin.json")) {
        return nullptr;
    }
    bool isImage = file.endsWith(".png") || file.endsWith(".jpg");
    if(!isImage) {
        return nullptr;
    }
    int pos = file.indexOf("skins");
    QString skinName = file.mid(pos + 6);
    pos = skinName.indexOf('/');
    QString uname = skinName.left(pos);
    if(!m_skins.contains(uname)) {
        return nullptr;
    }
    QString fileSkin = skinPath() + "/" + skinName.mid(pos+1);
    return new QFSFileEngine(fileSkin);
}

void QWoTheme::reload()
{
    QString pathApp = QCoreApplication::applicationDirPath();
    QStringList paths = m_customSkinPaths;
    QString path = QDir::cleanPath(pathApp + "/../private/skins");
    paths.append(path);
    m_skins.clear();
    for(auto it = paths.begin(); it != paths.end(); it++) {
        QString path = *it;
        QDir dir;
        if(dir.cd(path)) {
            QList<QFileInfo> fis = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
            for(int i = 0; i < fis.length(); i++) {
                QFileInfo& fi = fis[i];
                qDebug() << fi.fileName() << fi.absoluteFilePath();
                QString jsonFile = fi.absoluteFilePath() + "/skin.json";
                if(!QFile::exists(jsonFile)) {
                    continue;
                }
                QFile f(jsonFile);
                if(!f.open(QFile::ReadOnly)) {
                    continue;
                }
                QByteArray all = f.readAll();
                QJsonDocument jsdoc = QJsonDocument::fromJson(all);
                if(jsdoc.isEmpty()) {
                    continue;
                }
                QJsonObject root = jsdoc.object();
                QString desktop = root.value("desktop").toString();
                QJsonObject names = root.value("names").toObject();
                if(names.isEmpty() || desktop.isEmpty()) {
                    continue;
                }
                QString qssDesk = QDir::cleanPath(fi.absoluteFilePath() + "/" + desktop);
                if(!QFile::exists(qssDesk)) {
                    continue;
                }

                SkinData sd;
                sd.path = fi.absoluteFilePath();
                sd.qssFile = qssDesk;
                for(auto it = names.begin(); it != names.end(); it++) {
                    sd.names.insert(it.key(), it.value().toString());
                }
                m_skins.insert(fi.fileName(), sd);
            }
        }
    }
}

bool QWoTheme::loadSkinByUniqueName(const QString &name)
{
    if(!m_skins.contains(name)) {
        return false;
    }
    qDebug() << "loadSkinByUniqueName" << name;
    const SkinData& sd = m_skins.value(name);
    QFile f(sd.qssFile);
    if(!f.open(QFile::ReadOnly)) {
        return false;
    }
    QByteArray qss = f.readAll();
    f.close();
    QWoApplication::instance()->setStyleSheet(qss);
    return true;
}

QString QWoTheme::skinPath() const
{
    QString name = skinUniqueName();
    const SkinData& sd = m_skins.value(name);
    if(!sd.isValid()) {
        return QString();
    }
    return sd.path;
}
