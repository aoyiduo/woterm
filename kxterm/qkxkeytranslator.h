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

#ifndef QKEYTRANSLATOR_H
#define QKEYTRANSLATOR_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QKeySequence>

class QKxKeyTranslator : public QObject
{
    Q_OBJECT
public:    
    enum VTMode {
        //YES
        VM_ANSI_YES = 1 << 1, // vt100/vt102 is Ansi mode,but vt52 not Ansi mode.
        VM_NEWLINE_YES = 1 << 2,
        VM_APPCUKEY_YES = 1 << 3, // like [top], [tail -f] etc.
        VM_APPSCREEN_YES = 1 << 4, // like vim / vi.
        VM_APPKEYPAD_YES = 1 << 5, // num pad [0 ~ 9]number keyboard.
        //No
        VM_ANSI_NO = 1 << 8,
        VM_NEWLINE_NO = 1 << 9,
        VM_APPCUKEY_NO = 1 << 10,
        VM_APPSCREEN_NO = 1 << 11,
        VM_APPKEYPAD_NO = 1 << 12
    };
    Q_DECLARE_FLAGS(VTModes, VTMode)
    Q_FLAGS(VTModes)

    enum ModifyKey {
        //YES
        MK_SHIFT_YES = 1,
        MK_CTRL_YES = 1 << 1,
        MK_ALT_YES = 1 << 2,
        MK_META_YES = 1 << 3,
        MK_ANY_YES = 1 << 4,
        MK_KEYPAD_YES = 1 << 5,
        //NO
        MK_SHIFT_NO = 1 << 8,
        MK_CTRL_NO = 1 << 9,
        MK_ALT_NO = 1 << 10,
        MK_META_NO = 1 << 11,
        MK_ANY_NO = 1 << 12,
        MK_KEYPAD_NO = 1 << 13
    };
    Q_DECLARE_FLAGS(ModifyKeys, ModifyKey)
    Q_FLAGS(ModifyKeys)
    
    typedef struct KeyInfo {
        QByteArray  raw;
        ModifyKeys  modify;
        VTModes     mode;
        QByteArray  out;
        int         key;
    } KeyInfo;

public:
    explicit QKxKeyTranslator(QObject *parent = nullptr);
    QString path() const;
    QString name() const;
    void setName(const QString& name);
    bool load(const QString& file);
    QByteArray match(int key, Qt::KeyboardModifiers modifiers, VTModes modes);
private:
    bool parseKey(const QString&key, KeyInfo &ki);
    QStringList tokenArray(const QString& keys);
    void parseOutput(const QString& key, const QString& output);
    bool modifierEqual(ModifyKeys mk, Qt::KeyboardModifiers modifier);
private:
    QString m_path;
    QString m_title;
    QMap<int, QList<KeyInfo>> m_keys;
};

#endif // QKEYTRANSLATOR_H
