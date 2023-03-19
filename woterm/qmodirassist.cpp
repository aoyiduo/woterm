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

#include "qmodirassist.h"

#include <QUrl>
#include <QDebug>


QMoDirAssist::QMoDirAssist(QObject *parent)
    : QObject(parent)
    , m_filters(AllEntries|AllDirs|Drives)
{
    QFileInfoList lsfi = QDir::drives();
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        const QFileInfo& fi = *it;
        m_drivers.append(fi.absoluteFilePath());
    }
}

QStringList QMoDirAssist::nameFilters() const
{
    return m_nameFilters;
}

void QMoDirAssist::setNameFilters(const QStringList &filters)
{
    if(m_nameFilters != filters) {
        m_nameFilters = filters;
        emit nameFiltersChanged();
    }
}

QMoDirAssist::Filters QMoDirAssist::filter() const
{
    return m_filters;
}

void QMoDirAssist::setFilter(QMoDirAssist::Filters filters)
{
    m_filters = filters;
}

QString QMoDirAssist::homePath() const
{
    return QDir::homePath();
}

QVariantList QMoDirAssist::entryInfoList(const QString &_path) const
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

QString QMoDirAssist::absoluteFilePath(const QString &_path) const
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

QString QMoDirAssist::cleanPath(const QString &path) const
{
    return QDir::cleanPath(path);
}

QString QMoDirAssist::toNativeSeparators(const QString &path) const
{
    return QDir::toNativeSeparators(path);
}

QVariantMap QMoDirAssist::fileInfo(const QString &path) const
{
    QFileInfo fi(path);
    return castToVariantMap(fi);
}

bool QMoDirAssist::makePath(const QString &path)
{
    QDir d;
    return d.mkpath(path);
}

QVariantMap QMoDirAssist::castToVariantMap(const QFileInfo &fi) const
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
    dm.insert("canWrite", (flag & (QFile::WriteUser)) > 0);
    dm.insert("canRead", (flag & (QFile::ReadUser)) > 0);
    dm.insert("canExecute", (flag & (QFile::ExeUser)) > 0);
    return dm;
}

QVariantList QMoDirAssist::castToVariantList(const QFileInfoList &lsfi) const
{
    QVariantList lsv;
    for(auto it = lsfi.begin(); it != lsfi.end(); it++) {
        lsv.append(castToVariantMap(*it));
    }
    return lsv;
}

