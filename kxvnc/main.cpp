#include <QApplication>
#include <QtDebug>
#include <QIcon>
#include <QMenuBar>
#include <QProcess>
#include <QMessageBox>
#include <QList>
#include <QByteArray>
#include <QStyleFactory>
#include <QTextCodec>
#include <QMainWindow>


#include "qkxvncwidget.h"

void test() {
    QByteArray buf;
    buf = "123456";
    buf.reserve(1024);
    buf.resize(2048);
    int cap1 = buf.capacity();
    buf.resize(1024*10);
    int cap2 = buf.capacity();
    buf.resize(10);
    int cap3 = buf.capacity();
    buf.mid(0, 3);
    QByteArray ret = buf.remove(0, 3);
    int x = 5 % 3;
    qDebug() << ret << buf;

}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    static QApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    test();
    QMainWindow main;
    QKxVNCWidget vnc(&main);
    main.setCentralWidget(&vnc);
    main.resize(800, 600);
    main.show();
    return app.exec();
}
