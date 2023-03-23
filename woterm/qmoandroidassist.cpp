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

#include "qmoandroidassist.h"

#include "qkxmessagebox.h"


#include <QDir>
#include <QFile>

Q_GLOBAL_STATIC(QMoAndroidAssist, gAssist)

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>

static void messageFromJava(JNIEnv *env, jobject thiz, jstring value)
{
    QByteArray buf = env->GetStringUTFChars(value, nullptr);
    QMoAndroidAssist::instance()->onMessageFromJava(buf);
}

QMoAndroidAssist::QMoAndroidAssist(QObject *parent)
    : QObject(parent)
{
}

QMoAndroidAssist *QMoAndroidAssist::instance()
{
    return gAssist;
}

void QMoAndroidAssist::notify(const QString &title, const QString &msg)
{
    QAndroidJniObject jtitle = QAndroidJniObject::fromString(title);
    QAndroidJniObject jmsg = QAndroidJniObject::fromString(msg);
    QAndroidJniObject::callStaticMethod<void>("com/aoyiduo/woterm/MainActivity",
                                              "notify",
                                              "(Ljava/lang/String;Ljava/lang/String;)V",
                                              jtitle.object<jstring>(),
                                              jmsg.object<jstring>());
}

int QMoAndroidAssist::installAPK(const QString &fileApk)
{
    if(!QFile::exists(fileApk)) {
        return -1;
    }
#if 0
    // not any use..
    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>("com/aoyiduo/woterm/MainActivity", "canRequestPackageInstalls", "()Z");
    if(!ok) {
        QAndroidJniObject jintent = QAndroidJniObject::callStaticObjectMethod("com/aoyiduo/woterm/MainActivity", "requestPackageInstall", "()Landroid/content/Intent;");
        QAndroidIntent intent(jintent.object());
        QAndroidJniObject activity = QtAndroid::androidActivity();
        activity.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.handle().object());
    }
#endif
    QAndroidJniObject path = QAndroidJniObject::fromString(fileApk);
    jint ret = QAndroidJniObject::callStaticMethod<jint>("com/aoyiduo/woterm/MainActivity", "install", "(Ljava/lang/String;)I", path.object<jstring>());
    switch (ret) {
    case 0:
        return 0;
    case -1:
        return -2;
    case -2:
        return -3;

    }
    return -4;
}

void QMoAndroidAssist::onMessageFromJava(const QByteArray &msg)
{

}

void QMoAndroidAssist::init()
{
#if 0
    // it will cause error on x86 platform. that will happen on other platform or other version.
    try{
        JNINativeMethod methods[] {{"messageFromJava", "(Ljava/lang/String;)V", reinterpret_cast<void *>(messageFromJava)}};
        QAndroidJniObject javaClass("com/aoyiduo/woterm/MainActivity");

        QAndroidJniEnvironment env;
        jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
        if(objectClass) {
            env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
            env->DeleteLocalRef(objectClass);
        }
    }catch(...) {
        qDebug() << "Failed to init...";
    }
#endif
}

#else

QMoAndroidAssist::QMoAndroidAssist(QObject *parent)
    : QObject(parent)
{

}

QMoAndroidAssist *QMoAndroidAssist::instance()
{
    return gAssist;
}

void QMoAndroidAssist::notify(const QString &title, const QString &msg)
{

}

int QMoAndroidAssist::installAPK(const QString &fileApk)
{
    if(!QFile::exists(fileApk)) {
        return -1;
    }
    return -4;
}

void QMoAndroidAssist::onMessageFromJava(const QByteArray &msg)
{

}

void QMoAndroidAssist::init()
{

}

#endif
