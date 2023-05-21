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

#ifndef QWOTHEME_H
#define QWOTHEME_H

#include <QObject>
#include <QIcon>
#include <QPointer>
#include <QMap>

class QAbstractFileEngine;
class FileEngineHandler;
class QWoTheme : public QObject
{
    Q_OBJECT

    struct SkinData {
        QString path;
        QMap<QString, QString> names;
        QString qssFile;

        bool isValid() const {
            return !path.isEmpty();
        }
    };

public:
    explicit QWoTheme(QObject *parent = nullptr);
    virtual ~QWoTheme();

    static QWoTheme* instance();

    bool loadSkinByUniqueName(const QString& name);

    QString skinPath() const;
    QString skinUniqueName() const;
    void setSkinUniqueName(const QString& name);
    QString skinPathByUniqueName(const QString& name);
    QMap<QString,QString> skinFriendNames();
    QString skinFriendName(const QString& name);

    void addCustomSkinPaths(const QStringList& paths);
    static QIcon icon(const QString& fileName);
private:
    QAbstractFileEngine *create(const QString &file) const;
    void reload();

private:
    friend class FileEngineHandler;
    QString m_skinPath;
    FileEngineHandler *m_handler;
    QStringList m_customSkinPaths;
    QString m_skinUniqueName;
    QMap<QString, SkinData> m_skins;
};
#endif // QWOTHEME_H
