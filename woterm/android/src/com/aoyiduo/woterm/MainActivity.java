package com.aoyiduo.woterm;

import org.qtproject.qt5.android.QtNative;

import java.lang.String;
import java.io.File;
import android.os.Build;
import android.content.Intent;
import android.util.Log;
import android.net.Uri;
import android.content.ContentValues;
import android.content.Context;
import androidx.core.content.FileProvider;
import android.os.Environment;
import android.app.Notification;
import android.app.NotificationManager;

public class MainActivity extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static native void messageFromJava(String message);

    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;
    private static MainActivity m_context;

    public MainActivity()
    {
        m_context = this;
    }

    public static void notify(String title, String msg)
    {
        if (m_notificationManager == null) {
            m_notificationManager = (NotificationManager)m_context.getSystemService(Context.NOTIFICATION_SERVICE);
            m_builder = new Notification.Builder(m_context);
            m_builder.setSmallIcon(R.drawable.icon);
            m_builder.setContentTitle(title);
        }

        m_builder.setContentText(msg);
        m_notificationManager.notify(1, m_builder.build());
    }

    public static int install(String path) {
        if (QtNative.activity() == null){
            return -1;
        }
        File apk = new File(path);
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            //
            //
            // Note that the second parameter should be the same as the [android: authorities] value in the manifest
            // android:authorities="com.aoyiduo.woterm.fileProvider"
            //
            //String fileProvider = m_context.getPackageName() + ".fileProvider";
            //Uri uri = FileProvider.getUriForFile(m_context, fileProvider, apk);
            String fileProvider = QtNative.activity().getApplicationContext().getPackageName() + ".fileProvider";
            Uri uri = FileProvider.getUriForFile(QtNative.activity(), fileProvider, apk);
            intent.setDataAndType(uri, "application/vnd.android.package-archive");
        } else {
            intent.setDataAndType(Uri.fromFile(apk), "application/vnd.android.package-archive");
        }
        try {
            QtNative.activity().startActivity(intent);
            return 0;
        } catch(Exception e) {
            e.printStackTrace();
        }
        return -2;
    }
}
