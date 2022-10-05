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

#include <QApplication>
#include <QtDebug>
#include <QIcon>
#include <QMenuBar>
#include <QProcess>
#include <QMessageBox>
#include <QList>
#include <QByteArray>
#include <QStyleFactory>
#include <QSharedPointer>
#include <QPointer>
#include <QWeakPointer>

#include "qwoglobal.h"
#include "qwomainwindow.h"
#include "qwotermwidget.h"
#include "qwosetting.h"
#include "qwosshconf.h"
#include "qwotermstyle.h"
#include "qwoutils.h"
#include "qwossh.h"
#include "qkxutils.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
      qDebug() << "Debug: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtInfoMsg:
      qDebug() << "Info: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtWarningMsg:
      qDebug() << "Warning: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtCriticalMsg:
      qDebug() << "Critical: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  case QtFatalMsg:
      qDebug() << "Fatal: %s (%s:%u, %s)\n" << localMsg.constData() << context.file << context.line << context.function;
      break;
  }
}

void test()
{

}

int main(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageOutput);
#ifdef Q_OS_WIN
    static QApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    test();
    QStringList styles = QStyleFactory::keys();
    qDebug() << "embeded style list: " << styles;
    QApplication::setStyle(new QWoTermStyle("fusion"));
    //QApplication::setStyle("fusion");
    QFile f(":/woterm/resource/qss/default.qss");
    f.open(QFile::ReadOnly);
    QByteArray qss = f.readAll();
    f.close();
    app.setStyleSheet(qss);


    QApplication::setWindowIcon(QIcon(":/woterm/resource/skin/woterm4.png"));

    QString path = QApplication::applicationDirPath();
    QApplication::addLibraryPath(path);
    QString libPath = QDir::cleanPath(path + "/../lib");
    if(QFile::exists(libPath)) {
        QApplication::addLibraryPath(libPath);
    }

    QTranslator translator;
    QString lang = QWoSetting::languageFile();
     if(!lang.isEmpty() && translator.load(lang)){
        app.installTranslator(&translator);
    }

    QWoMainWindow *pmw = QWoMainWindow::instance();
    pmw->show();
    QTimer::singleShot(0, pmw, SLOT(onAppStart()));
    int code = app.exec();
    return code;
}
