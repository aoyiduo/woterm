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

#ifndef QWOPLAYBOOKWIDGET_H
#define QWOPLAYBOOKWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QMessageBox>
#include <QVariant>
#include <QJSValue>
#include <QQmlError>
#include <QList>

namespace Ui {
class QWoPlaybookWidget;
}

class QQmlContext;
class QQuickItem;
class QFileSystemWatcher;
class QSettings;
class QQuickWidget;
class QSplitter;
class QWoPlaybookTermWidget;
class QWoPlaybookConsoleProxy;

class QWoPlaybookWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor() WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)
public:
    explicit QWoPlaybookWidget(const QString& path, const QString& name, QWidget *parent = nullptr);
    virtual ~QWoPlaybookWidget();

    Q_INVOKABLE bool isConsoleVisible();
    Q_INVOKABLE void openConsole();
    Q_INVOKABLE void closeConsole();

    Q_INVOKABLE void init(QObject *loader);
    Q_INVOKABLE QString hostChooser();

    Q_INVOKABLE int critical(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int information(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int question(const QString &title, const QString &text, int buttons = int(QMessageBox::Yes | QMessageBox::No), int defaultButton = QMessageBox::NoButton);
    Q_INVOKABLE int warning(const QString &title, const QString &text, int buttons = QMessageBox::Ok, int defaultButton = QMessageBox::NoButton);

    Q_INVOKABLE QVariant storageValue(const QString& key, const QVariant& vdef=QVariant());
    Q_INVOKABLE void setStorageValue(const QString& key, const QVariant& val);

    Q_INVOKABLE QString urlToLocalFile(const QString& url);
    Q_INVOKABLE void openTerminal();
    Q_INVOKABLE void closeTerminal();
    Q_INVOKABLE void adjustTerminal();
    Q_INVOKABLE QObject *terminal();
    Q_INVOKABLE QString log(const QJSValue& v1,
                         const QJSValue& v2=QJSValue(),
                         const QJSValue& v3=QJSValue(),
                         const QJSValue& v4=QJSValue(),
                         const QJSValue& v5=QJSValue(),
                         const QJSValue& v6=QJSValue(),
                         const QJSValue& v7=QJSValue(),
                         const QJSValue& v8=QJSValue(),
                         const QJSValue& v9=QJSValue());
    Q_INVOKABLE void logClear();

    bool isDebugMode();
    void setDebugMode(bool on);

    // background color
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& clr);
    //
    QColor textColor() const;
    void setTextColor(const QColor& clr);

    QColor highlightColor() const;
    void setHighlightColor(const QColor& clr);

signals:
    void backgroundColorChanged();
    void textColorChanged();
    void highlightColorChanged();

private slots:
    void onBrowserScriptButtonClicked();
    void onReloadButtonClicked();
    void onWindowCloseArrived();
    void onScriptEditModeButtonClicked();
    void onScriptCloseButtonClicked();
    void onWatchDirectoryChanged(const QString& path);
    void onEngineWarnings(const QList<QQmlError> &warnings);
private:
    Q_INVOKABLE void showMessage(const QString& title, const QString &msg);
    Q_INVOKABLE void initAsync(QObject* loader);
    Q_INVOKABLE void reload();
    void resetStatus();
    QSettings *settingGet();
    QRegion mapToRoot(QQuickItem *item);
private:
    virtual void paintEvent(QPaintEvent *ev);
private:
    Ui::QWoPlaybookWidget *ui;
    QPointer<QQmlContext> m_qmlContext;
    QPointer<QQuickItem> m_loader;
    QPointer<QFileSystemWatcher> m_watcher;
    QPointer<QSplitter> m_root;
    QPointer<QQuickWidget> m_quick;
    QPointer<QWoPlaybookTermWidget> m_term;
    QString m_path, m_qmlFile;
    QString m_name;
    QColor m_backgroundColor, m_textColor, m_highlightColor;
    QPointer<QSettings> m_settings;

    QPointer<QWoPlaybookConsoleProxy> m_consoleProxy;
};

#endif // QWOPLAYBOOKWIDGET_H
