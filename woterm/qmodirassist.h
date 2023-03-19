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

#ifndef QMODIRASSIST_H
#define QMODIRASSIST_H

#include <QObject>
#include <QDir>
#include <QFileInfoList>
#include <QList>
#include <QVariantMap>

class QMoDirAssist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QMoDirAssist::Filters filter READ filter WRITE setFilter)
public:
    enum Filter {
        Dirs        = QDir::Dirs,
        Files       = QDir::Files,
        Drives      = QDir::Drives,
        NoSymLinks  = QDir::NoSymLinks,
        AllEntries  = QDir::AllEntries,
        TypeMask    = QDir::TypeMask,

        Readable    = QDir::Readable,
        Writable    = QDir::Writable,
        Executable  = QDir::Executable,
        PermissionMask    = QDir::PermissionMask,

        Modified    = QDir::Modified,
        Hidden      = QDir::Hidden,
        System      = QDir::System,

        AccessMask  = QDir::AccessMask,

        AllDirs       = QDir::AllDirs,
        CaseSensitive = QDir::CaseSensitive,
        NoDot         = QDir::NoDot,
        NoDotDot      = QDir::NoDotDot,
        NoDotAndDotDot = QDir::NoDotAndDotDot,

        NoFilter = QDir::NoFilter
    };
    Q_DECLARE_FLAGS(Filters, Filter)

public:
    explicit QMoDirAssist(QObject *parent = nullptr);

    QStringList nameFilters() const;
    void setNameFilters(const QStringList& filters);

    QMoDirAssist::Filters filter() const;
    void setFilter(QMoDirAssist::Filters filters);

    Q_INVOKABLE QString homePath() const;
    Q_INVOKABLE QVariantList entryInfoList(const QString& path = QString()) const;
    Q_INVOKABLE QString absoluteFilePath(const QString& path) const;
    Q_INVOKABLE QString cleanPath(const QString& path) const;
    Q_INVOKABLE QString toNativeSeparators(const QString& path) const;
    Q_INVOKABLE QVariantMap fileInfo(const QString& path) const;
    Q_INVOKABLE bool makePath(const QString& path);
signals:
    void nameFiltersChanged();
private:
    QVariantMap castToVariantMap(const QFileInfo& fi) const;
    QVariantList castToVariantList(const QFileInfoList& lsfi) const;

private:
    QStringList m_nameFilters;
    QMoDirAssist::Filters m_filters;
    QStringList m_drivers;
};

#endif // QMODIRASSIST_H
