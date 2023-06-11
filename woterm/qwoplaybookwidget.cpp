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

#include "qwoplaybookwidget.h"
#include "ui_qwoplaybookwidget.h"
#include "qwosetting.h"
#include "qkxmessagebox.h"
#include "qwosshconf.h"
#include "qwohostsimplelist.h"
#include "qkxscriptremotecommand.h"
#include "qwoutils.h"
#include "qwoplaybooktermwidget.h"
#include "qwoplaybookconsole.h"
#include "qwoplaybookconsoleproxy.h"

#include <QFileDialog>
#include <QProcess>
#include <QJsonDocument>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QFileSystemWatcher>
#include <QStyleOption>
#include <QStyle>
#include <QPainter>
#include <QSettings>
#include <QDesktopServices>
#include <QQuickWidget>
#include <QSplitter>


QWoPlaybookWidget::QWoPlaybookWidget(const QString &path, const QString& name, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWoPlaybookWidget)
    , m_path(path)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);

    QLayout *vbox = layout();
    m_root = new QSplitter(Qt::Vertical, this);
    vbox->addWidget(m_root);
    m_quick = new QQuickWidget(m_root);
    m_root->addWidget(m_quick);
    m_quick->setMinimumHeight(100);

    m_quick->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quick->setAttribute(Qt::WA_TranslucentBackground);
    m_quick->setClearColor(Qt::black);

    QObject::connect(ui->btnBrowserScript, SIGNAL(clicked()), this, SLOT(onBrowserScriptButtonClicked()));
    QObject::connect(ui->btnReload, SIGNAL(clicked()), this, SLOT(onReloadButtonClicked()));
    QObject::connect(ui->radAuto, SIGNAL(clicked()), this, SLOT(onScriptEditModeButtonClicked()));
    QObject::connect(ui->radManual, SIGNAL(clicked()), this, SLOT(onScriptEditModeButtonClicked()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(onScriptCloseButtonClicked()));

    m_consoleProxy = new QWoPlaybookConsoleProxy(name, this);

    QQmlEngine *engine = m_quick->engine();
    QWoUtils::injectJS(engine, QStringList() << ":/woterm/js/async.js" << ":/woterm/js/utils.js" );
    QObject::connect(engine, SIGNAL(quit()), this, SLOT(onWindowCloseArrived()));
    QObject::connect(engine, SIGNAL(warnings(QList<QQmlError>)), this, SLOT(onEngineWarnings(QList<QQmlError>)));

    QStringList gSessionNames = QWoSshConf::instance()->hostNameList(SshWithSftp);
    m_qmlContext = m_quick->rootContext();
    m_qmlContext->setContextProperty("gSessions", gSessionNames);
    m_qmlContext->setContextProperty("Playbook", this);
    m_qmlContext->setContextProperty("console", m_consoleProxy);
    m_quick->setSource(QUrl("qrc:/woterm/qml/playbook.qml"));
    ui->debugArea->setVisible(false);
}

QWoPlaybookWidget::~QWoPlaybookWidget()
{
    delete ui;
}

bool QWoPlaybookWidget::isConsoleVisible()
{
    return m_consoleProxy->isVisible();
}

void QWoPlaybookWidget::openConsole()
{
    m_consoleProxy->open();
}

void QWoPlaybookWidget::closeConsole()
{
    m_consoleProxy->close();
}

void QWoPlaybookWidget::init(QObject *loader)
{
    QMetaObject::invokeMethod(this, "initAsync", Qt::QueuedConnection, Q_ARG(QObject*, loader));
}

QString QWoPlaybookWidget::hostChooser()
{
    QWoHostSimpleList dlg(SshWithSftp, this);
    dlg.setWindowTitle(tr("SSH host chooser"));
    if(dlg.exec() == QDialog::Accepted+1) {
        HostInfo hi;
        dlg.result(&hi);
        return hi.name;
    }
    return "";
}

int QWoPlaybookWidget::critical(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::critical(this, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QWoPlaybookWidget::information(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::information(this, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QWoPlaybookWidget::question(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::question(this, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

int QWoPlaybookWidget::warning(const QString &title, const QString &text, int buttons, int defaultButton)
{
    return QKxMessageBox::warning(this, title, text, QMessageBox::StandardButtons(buttons), QMessageBox::StandardButton(defaultButton));
}

QVariant QWoPlaybookWidget::storageValue(const QString &key, const QVariant &vdef)
{
    return settingGet()->value(key, vdef);
}

void QWoPlaybookWidget::setStorageValue(const QString &key, const QVariant &val)
{
    settingGet()->setValue(key, val);
}

QString QWoPlaybookWidget::urlToLocalFile(const QString &url)
{
    if(url.startsWith("file://")) {
        QUrl tmp(url);
        return tmp.toLocalFile();
    }
    return url;
}

void QWoPlaybookWidget::openTerminal()
{
    if(m_term == nullptr) {
        m_term = new QWoPlaybookTermWidget(this);
        QQmlEngine *engine = m_quick->engine();
        if(engine) {
            engine->setProperty("terminal", QVariant::fromValue<QObject*>(m_term));
        }
        m_root->addWidget(m_term);
    }
}

void QWoPlaybookWidget::closeTerminal()
{
    if(m_term) {
        m_term->deleteLater();
    }
}

void QWoPlaybookWidget::adjustTerminal()
{
    QQuickItem *root = m_quick->rootObject();
    QRegion rgn = mapToRoot(root);
    QRect rt = rgn.boundingRect();
    int height = m_root->height();
    int whdl = m_root->handleWidth();
    //qDebug() << "adjustTerminal" << rt << height << whdl;
    QList<int> lsize;
    lsize << rt.height() << height - rt.height() - whdl;
    m_root->setSizes(lsize);
}

QObject *QWoPlaybookWidget::terminal()
{
    return m_term;
}

QString QWoPlaybookWidget::log(const QJSValue &v1, const QJSValue &v2, const QJSValue &v3, const QJSValue &v4, const QJSValue &v5, const QJSValue &v6, const QJSValue &v7, const QJSValue &v8, const QJSValue &v9)
{
    QJSValueList all = {v1, v2, v3, v4, v5, v6, v7, v8, v9};
    QStringList oks;
    for(auto it = all.begin(); it != all.end(); it++) {
        QJSValue v = *it;
        if(v.isUndefined()) {
            continue;
        }
        oks.append(v.toString());
    }
    QString msg = oks.join(',');
    msg.append("\r\n");
    if(m_term) {
        m_term->append(msg);
        m_term->scrollToEnd();
    }
    return msg;
}

void QWoPlaybookWidget::logClear()
{
    if(m_term) {
        m_term->clear();
    }
}

bool QWoPlaybookWidget::isDebugMode()
{
    return ui->debugArea->isVisible();
}

void QWoPlaybookWidget::setDebugMode(bool on)
{
    ui->debugArea->setVisible(on);
    resetStatus();
}

QColor QWoPlaybookWidget::backgroundColor() const
{
    return m_backgroundColor;
}

void QWoPlaybookWidget::setBackgroundColor(const QColor &clr)
{
    if(m_backgroundColor != clr) {
        m_backgroundColor = clr;
        emit backgroundColorChanged();
    }
}

QColor QWoPlaybookWidget::textColor() const
{
    return m_textColor;
}

void QWoPlaybookWidget::setTextColor(const QColor &clr)
{
    if(m_textColor != clr) {
        m_textColor = clr;
        emit textColorChanged();
    }
}

QColor QWoPlaybookWidget::highlightColor() const
{
    return m_highlightColor;
}

void QWoPlaybookWidget::setHighlightColor(const QColor &clr)
{
    if(m_highlightColor != clr) {
        m_highlightColor = clr;
        emit highlightColorChanged();
    }
}

void QWoPlaybookWidget::onBrowserScriptButtonClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_path));
}

void QWoPlaybookWidget::onReloadButtonClicked()
{
    reload();
}

void QWoPlaybookWidget::onWindowCloseArrived()
{
    // close();
}

void QWoPlaybookWidget::onScriptEditModeButtonClicked()
{
    resetStatus();
}

void QWoPlaybookWidget::onScriptCloseButtonClicked()
{
    setDebugMode(false);
}

void QWoPlaybookWidget::onWatchDirectoryChanged(const QString& path)
{
    reload();
}

void QWoPlaybookWidget::onEngineWarnings(const QList<QQmlError> &warnings)
{
    int idx = 0;
    if(ui->debugArea->isVisible()) {
        if(m_consoleProxy) {
            m_consoleProxy->open();
        }
    }
    for(auto it = warnings.begin(); it != warnings.end(); it++) {
        QQmlError err = *it;
        if(m_consoleProxy) {
            m_consoleProxy->log(err.toString());
        }
    }

}

void QWoPlaybookWidget::showMessage(const QString& title, const QString &msg)
{
    QKxMessageBox::warning(this,  title, msg);
}

void QWoPlaybookWidget::initAsync(QObject *loader)
{
    m_loader = qobject_cast<QQuickItem*>(loader);
    QString file = m_path + "/playbook.json";
    QFile f(file);
    if(!f.open(QFile::ReadOnly)) {
        QMetaObject::invokeMethod(this, "showMessage", Qt::QueuedConnection, Q_ARG(QString, tr("File error")), Q_ARG(QString, tr("The follow configure file is not exist.%1").arg(file)));
        return;
    }
    QJsonDocument jsdoc = QJsonDocument::fromJson(f.readAll());
    if(jsdoc.isEmpty()) {
        QMetaObject::invokeMethod(this, "showMessage", Qt::QueuedConnection, Q_ARG(QString, tr("File error")), Q_ARG(QString, tr("The follow configure file is not a json file.")));
        return;
    }

    if(!jsdoc.isObject()){
        return;
    }
    m_qmlFile = m_path + "/" + jsdoc["source"].toString();
    if(!QFile::exists(m_qmlFile)) {
        QMetaObject::invokeMethod(this, "showMessage", Qt::QueuedConnection, Q_ARG(QString, tr("File error")), Q_ARG(QString, tr("The follow configure file is not exist.%1").arg(m_qmlFile)));
    }
    m_name = jsdoc["name"].toString();
    loader->setProperty("source", QUrl::fromLocalFile(m_qmlFile));
}

void QWoPlaybookWidget::reload()
{
    QQmlEngine *engine = m_qmlContext->engine();
    engine->clearComponentCache();
    if(m_loader) {
        m_loader->setProperty("source", "");
        m_loader->setProperty("source", QUrl::fromLocalFile(m_qmlFile));
    }
}

void QWoPlaybookWidget::resetStatus()
{
    if(ui->radAuto->isChecked()) {
        if(m_watcher == nullptr) {
            m_watcher = new QFileSystemWatcher(this);
            QObject::connect(m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(onWatchDirectoryChanged(QString)));
            m_watcher->addPath(m_path);
        }
    }else{
        m_watcher->deleteLater();
    }
    reload();
}

QSettings *QWoPlaybookWidget::settingGet()
{
    if(m_settings == nullptr) {
        QFileInfo fi(m_path);
        QString fileName = fi.fileName();
        QString cfgFile = QWoSetting::ensurePath("playbook") + QString("/%1.ini").arg(fileName);
        m_settings = new QSettings(cfgFile);
    }
    return m_settings;
}

QRegion QWoPlaybookWidget::mapToRoot(QQuickItem *item)
{
    QQuickItem *root = m_quick->rootObject();
    QRectF rt(0, 0, item->width(), item->height());
    rt = item->mapRectToItem(root, rt);
    QRegion rgn;
    rgn += rt.toRect();
    QList<QQuickItem*> items = item->childItems();
    for(auto it = items.begin(); it != items.end(); it++) {
        QQuickItem *item = *it;
        if(!item->isVisible()) {
            continue;
        }
        rgn += mapToRoot(item);
    }
    return rgn;
}

void QWoPlaybookWidget::paintEvent(QPaintEvent *ev)
{
    QStyleOption o;
    o.initFrom(this);
    setBackgroundColor(o.palette.background().color());
    setTextColor(o.palette.foreground().color());
    setHighlightColor(o.palette.highlight().color());
    QWidget::paintEvent(ev);
}
