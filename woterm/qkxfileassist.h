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

#ifndef QKXFILEASSIST_H
#define QKXFILEASSIST_H

#include <QObject>
#include <QPointer>

class QFile;
class QKxFileAssist : public QObject
{
    Q_OBJECT
public:
    enum FileError {
        NoError = 0,
        ReadError = 1,
        WriteError = 2,
        FatalError = 3,
        ResourceError = 4,
        OpenError = 5,
        AbortError = 6,
        TimeOutError = 7,
        UnspecifiedError = 8,
        RemoveError = 9,
        RenameError = 10,
        PositionError = 11,
        ResizeError = 12,
        PermissionsError = 13,
        CopyError = 14
    };
    Q_ENUM(FileError)
public:
    explicit QKxFileAssist(QObject *parent = nullptr);

    Q_INVOKABLE bool rename(const QString& fileSrc, const QString& fileDst);
    Q_INVOKABLE bool exist(const QString& path) const;
    Q_INVOKABLE void close();
    Q_INVOKABLE bool open(const QString& path, bool readOnly);
    Q_INVOKABLE QByteArray read(int size);
    Q_INVOKABLE bool write(const QByteArray& buf);
    Q_INVOKABLE bool seek(qint64 pos);
    Q_INVOKABLE qint64 fileSize() const;
    Q_INVOKABLE FileError error() const;
    Q_INVOKABLE QString errorString() const;

private:
    QPointer<QFile> m_file;
};

#endif // QKXFILEASSIST_H
