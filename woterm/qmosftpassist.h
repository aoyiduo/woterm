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

#ifndef QMOSFTPASSIST_H
#define QMOSFTPASSIST_H

#include "qwoglobal.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QFileInfo>

class QWoSshFtp;
class QMoSftpRemoteModel;
class QWidget;
class QMessageBox;
class QMoSftpTransferAssist;
class QMoSftpQueueModel;

class QMoSftpAssist : public QObject
{
    Q_OBJECT
public:
    explicit QMoSftpAssist(const QString& target, int gid, QMoSftpRemoteModel *model, QWidget* widget, QObject *parent = nullptr);
    Q_INVOKABLE void openHome();
    Q_INVOKABLE void openDir(const QStringList& paths);
    Q_INVOKABLE void openDir(const QString& path);
    Q_INVOKABLE void mkDir(const QString &path, int mode);
    Q_INVOKABLE void rmDir(const QString &path);
    Q_INVOKABLE void unlink(const QString &path);
signals:
    void dirOpen(const QString &path, const QVariantList &v, const QVariantMap& userData);
    void commandStart(int type, const QVariantMap& v);
    void commandFinish(int type, const QVariantMap& v);
    void inputArrived(const QString &title, const QString &prompt, bool visible);
    void closeArrived();
private slots:
    /*dir or file.*/
    void onCommandStart(int type, const QVariantMap& userData);
    void onCommandFinish(int type, const QVariantMap& userData);
    void onConnectionStart();
    void onConnectionFinished(bool ok);
    void onFinishArrived(int code);
    void onErrorArrived(const QString& err, const QVariantMap& userData);
    void onPasswordArrived(const QString& host, const QByteArray& pass);
    void onPasswordInputResult(const QString& pass, bool isSave);
private:
    Q_INVOKABLE void reconnect();
    void release();
private:
    QString m_target;
    int m_gid;
    QPointer<QWoSshFtp> m_sftp;
    QPointer<QMoSftpRemoteModel> m_model;
    QPointer<QWidget> m_widget;
    QPointer<QMessageBox> m_dlgConfirm;
    QPointer<QMoSftpTransferAssist> m_transfer;
    bool m_savePassword;
};

#endif // QMOSFTPASSIST_H
