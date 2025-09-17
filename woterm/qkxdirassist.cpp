/*******************************************************************************************
*
* Copyright (C) 2023 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#include "qkxdirassist.h"

#include <QUrl>
#include <QDebug>
#include <QStandardPaths>


QKxDirAssist::QKxDirAssist(QObject *parent)
    : QObject(parent)
    , m_filters(AllEntries|AllDirs|Drives)
{
    QFileInfoList lsfi = QDir::drives();
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        const QFileInfo& fi = *it;
        m_drivers.append(fi.absoluteFilePath());
    }
}

QStringList QKxDirAssist::nameFilters() const
{
    return m_nameFilters;
}

void QKxDirAssist::setNameFilters(const QStringList &filters)
{
    if(m_nameFilters != filters) {
        m_nameFilters = filters;
        emit nameFiltersChanged();
    }
}

QKxDirAssist::Filters QKxDirAssist::filter() const
{
    return m_filters;
}

void QKxDirAssist::setFilter(QKxDirAssist::Filters filters)
{
    m_filters = filters;
}

QString QKxDirAssist::downloadLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}

QString QKxDirAssist::picturesLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

QString QKxDirAssist::appConfigLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString QKxDirAssist::appDataLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString QKxDirAssist::homeLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString QKxDirAssist::tempLocation() const
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

QString QKxDirAssist::homePath() const
{
    return QDir::homePath();
}

QVariantList QKxDirAssist::entryInfoList(const QString &_path) const
{
    QString path = absoluteFilePath(_path);
    if(path == "/") {
#if defined(Q_OS_WIN)
        QFileInfoList lsfi = QDir::drives();
#elif defined(Q_OS_UNIX)
        QDir d = QDir::root();
        int flags = m_filters;
        QFileInfoList lsfi = d.entryInfoList(m_nameFilters, (QDir::Filters)flags, QDir::DirsFirst|QDir::Name);
#endif
        return castToVariantList(lsfi);
    }
    QDir d(path);
    int flags = m_filters;
    QFileInfoList lsfi = d.entryInfoList(m_nameFilters, (QDir::Filters)flags, QDir::DirsFirst|QDir::Name);
    return castToVariantList(lsfi);
}

QString QKxDirAssist::absoluteFilePath(const QString &_path) const
{
    QString path = _path;
    if(path.startsWith("file:")) {
        QUrl url(path);
        path = url.toLocalFile();
    }
    if(path.isEmpty() || path == "/") {
        return "/";
    }
    if(path.at(0) == "~") {
        path.replace(0, 1, QDir::homePath());
    }
    QFileInfo fi(path);
    QString pathAbs = fi.absoluteFilePath();
    if(m_drivers.contains(pathAbs) && path.length() > 3) {
        QString tmp = path.mid(3);
        tmp.remove('/').remove('.').remove('\\');
        if(tmp.isEmpty()) {
            return "/";
        }
    }
    return pathAbs;
}

QString QKxDirAssist::cleanPath(const QString &path) const
{
    return QDir::cleanPath(path);
}

QString QKxDirAssist::toNativeSeparators(const QString &path) const
{
    return QDir::toNativeSeparators(path);
}

QVariantMap QKxDirAssist::fileInfo(const QString &path) const
{
    QFileInfo fi(path);
    return castToVariantMap(fi);
}

bool QKxDirAssist::makePath(const QString &path)
{
    QDir d;
    return d.mkpath(path);
}

bool QKxDirAssist::exist(const QString &path)
{
    QFileInfo fi(path);
    return fi.exists();
}

bool QKxDirAssist::rename(const QString &pathSrc, const QString &pathDst)
{
    QDir d;
    return d.rename(pathSrc, pathDst);
}

QVariantMap QKxDirAssist::castToVariantMap(const QFileInfo &fi) const
{
    QVariantMap dm;
    QString fileName = fi.fileName();
    if(fileName.isEmpty()) {
        fileName = fi.path();
    }
    dm.insert("path", fi.absolutePath()); // not include file, just parent path.
    dm.insert("fileName", fileName);
    dm.insert("filePath", fi.absoluteFilePath()); // include file path.
    dm.insert("isDir", fi.isDir());
    dm.insert("isRoot", fi.isRoot());
    dm.insert("isFile", fi.isFile());
    dm.insert("size", fi.size());
    QFile::Permissions flag = fi.permissions();
    QString permissions;
    permissions.append(fi.isDir() ? "d" : "-");
    permissions.append(fi.permission(QFile::ReadUser) ? "r" : "-");
    permissions.append(fi.permission(QFile::WriteUser) ? "w" : "-");
    permissions.append(fi.permission(QFile::ExeUser) ? "x" : "-");
    permissions.append(fi.permission(QFile::ReadGroup) ? "r" : "-");
    permissions.append(fi.permission(QFile::WriteGroup) ? "w" : "-");
    permissions.append(fi.permission(QFile::ExeGroup) ? "x" : "-");
    permissions.append(fi.permission(QFile::ReadOther) ? "r" : "-");
    permissions.append(fi.permission(QFile::WriteOther) ? "w" : "-");
    permissions.append(fi.permission(QFile::ExeOther) ? "x" : "-");
    dm.insert("permissions", permissions);
    dm.insert("canWrite", bool((flag & (QFile::WriteUser)) > 0));
    dm.insert("canRead", bool((flag & (QFile::ReadUser)) > 0));
    dm.insert("canExecute", bool((flag & (QFile::ExeUser)) > 0));
    return dm;
}

QVariantList QKxDirAssist::castToVariantList(const QFileInfoList &lsfi) const
{
    QVariantList lsv;
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        lsv.append(castToVariantMap(*it));
    }
    return lsv;
}

