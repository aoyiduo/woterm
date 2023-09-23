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

#ifndef QWOSETTING_H
#define QWOSETTING_H

#include "qkxsetting.h"

#include <QVariant>
#include <QDate>

class QWoSetting : public QKxSetting
{
    Q_OBJECT
public:
    explicit QWoSetting(QObject *parent = nullptr);
    static QString ftpTaskPath();
    static QString identityFilePath();
    static QString historyFilePath();
    static QString examplePath();
    static QString privatePath();
    static QString privateLanguagesPath();
    static QString sshServerFilePath();
    static QString lastJsLoadPath();
    static void setLastJsLoadPath(const QString& path);
    static QMap<QString, QString> allLanguages();
    Q_INVOKABLE static QString languageTypeAsBCP47Field();
    Q_INVOKABLE static QStringList allLanguageNames();
    Q_INVOKABLE static QString languagePath(const QString& name);
    Q_INVOKABLE static QString languageName(const QString& path);
    Q_INVOKABLE static QString languageFileName();
    Q_INVOKABLE static QString absoluteLanguageFilePath();
    Q_INVOKABLE static void setLanguageFileName(const QString& path);
    Q_INVOKABLE static bool isChineseLanguageFile();

    static QString playbooksPath();
    static QString customPlaybooksPath();
    static void setCustomPlaybooksPath(const QString& path);

    static QString sshServerDbPath();
    static QString sftpTaskDbPath();
    static QString sftpTaskLogPath();
    static QString tempPath();
    static QString fontBackupPath();
    static QString cachePath();
    static QString viewPath();
    static QString fileIconCachePath();

    Q_INVOKABLE static QString downloadPath();
    Q_INVOKABLE static void setDownloadPath(const QString& path);
    Q_INVOKABLE static QString lastBackupPath();
    Q_INVOKABLE static void setLastBackupPath(const QString& path);

    /* upgrade */
    static bool shouldReportLicense();
    static void setIgnoreTodayReportLicense();

    static bool isAlwayFreeVersion();
    static void setAlwayFreeVersion(bool on);

    static bool shouldPopupUpgradeUltimate();
    static void setIgnoreTodayUpgradeUltimate();

    static bool shouldPopupUpgradeVersionMessage(const QString& ver);
    static void setNextUpgradeVersionDate(const QString& ver, const QDate &dt);
    static void setIgnoreTodayUpgradeVersion(const QString& ver);
    static void setSkipThisVersion(const QString& ver);

    static bool isListModel(const QString& where);
    static void setListModel(const QString& where, bool isList);

    static QString adminPassword();
    static void setAdminPassword(const QString& pass);

    static QStringList sessionsGroupExpand(const QString& where);
    static void setSessionsGroupExpand(const QString& where, const QStringList& expands);

    static bool startupByAdmin();
    static void setStartupByAdmin(bool on);

    static bool allowOtherHostConnectToTunnel();
    static void setAllowOtherHostConnectToTunnel(bool on);

    static bool tunnelRunAsDaemon();
    static void setTunnelRunAsDaemon(bool on);

    static bool lookupPasswordByAdmin();
    static void setLookupPasswordByAdmin(bool on);

    static bool localTerminalOnAppStart();
    static void setLocalTerminalOnAppStart(bool on);

    static int windownOpacity();
    static void setWindowOpacity(int level);
    static bool windownOpacityEnable();
    static void setWindowOpacityEnable(bool on);
    static bool allowToSetWindowOpacity();
    static void setAllowToSetWindowOpacity(bool on);

    static bool serialportOnAppStart();
    static void setSerialportOnAppStart(bool on);

    static QVariantMap localShell();
    static void setLocalShell(const QVariantMap& dm);

    static QVariantMap serialPort();
    static void setSerialPort(const QVariantMap& dm);

    static QVariantMap rdpDefault();
    static void setRdpDefault(const QVariantMap& dm);

    static QVariantMap vncDefault();
    static void setVncDefault(const QVariantMap& dm);

    static QVariantMap ttyDefault();
    static void setTtyDefault(const QVariantMap& dm);

    static bool tunnelMultiplex();
    static void setTunnelMultiplex(bool on);
    static int tunnelMultiplexMaxCount();
    static void setTunnelMultipleMaxCount(int cnt);

    static QString terminalBackgroundImage();
    static void setTerminalBackgroundImage(const QString& path);
    static int terminalBackgroundImageAlpha();
    static void setTerminalBackgroundImageAlpha(int v);
    static bool terminalBackgroundImageEdgeSmooth();
    static void setTerminalBackgroundImageEdgeSmooth(bool v);
    static QString terminalBackgroundImagePosition();
    static void setTerminalBackgroundImagePosition(const QString& pos);

    // identity files
    static bool allowToUseExternalIdentityFiles();
    static void setAllowToUseExternalIdentityFiles(bool on);
    static bool enableUserHomeIdentityFiles();
    static void setEnableUserHomeIdentityFiles(bool on);
    static bool enableLocalSshAgent();
    static void setEnableLocalSshAgent(bool on);
    static bool enableRemoteSshAgent();
    static void setEnableRemoteSshAgent(bool on);
    static QString remoteSshAgentAddress();
    static void setRemoteSshAgentAddress(const QString& addr);
    static QString lastLocalSshAgentAddress();
    static void setLastLocalSshAgentAddress(const QString& addr);
    static QByteArray sshAuthSockDefault();

    static bool allowSftpToOpenFile();
    static void setAllowSftpToOpenFile(bool on);

};

#endif
