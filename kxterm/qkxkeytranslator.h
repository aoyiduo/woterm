/*******************************************************************************************
*
* Copyright (C) 2022 Guangzhou AoYiDuo Network Technology Co.,Ltd. All Rights Reserved.
*
* Contact: http://www.aoyiduo.com
*
*   this file is used under the terms of the Apache License, Version 2.0
* more information follow the website: https://www.apache.org/licenses/LICENSE-2.0.txt
*
*******************************************************************************************/

#ifndef QKEYTRANSLATOR_H
#define QKEYTRANSLATOR_H

#include "qvtedef.h"

#include <QObject>
#include <QMap>
#include <QList>
#include <QKeySequence>


/***
 *
 * "key" Keyname { ("+"|"-") Modename } ":" (String|Operation)
 *
 * extract from ${woterm}/private/keytabs/keyinfo.txt.
 * ${woterm}/private/keytabs/README-KeyTab
 * 需要认真理解keyinfo.txt和README-KeyTab的描述。
 *   需要区分Ansi模式，VT52 escape和ANSI escape的区别。
 *   需要区分AnyMod和Alt, Ctrl, Shift的区别。
 *   需要区分NewLine和发送\r\n和\r的关系。
 *
 *
 *  AnyModifier: If this mode is set, the key combination uses any modifier key (any of the previous three modifier keys: alt/ctrl/shift); and vice versa if it's reset
 *  Ansi: If this mode is set, &konsole; will send ANSI escape and control sequences
 *        If this mode is reset &konsole; will send VT52 escape and control sequences
 *  AppScreen: If this mode is set, the key combination will only affect interactive programs that use the Alternate Screen buffer, like vim.
 *             &konsole; makes use of two screen buffers:
 *             The Normal Screen buffer (default): allows you to scroll back to view previous lines of output, this is the default buffer you usually use to execute commands... &etc;
 *             The Alternate Screen buffer: the terminal switches to this buffer when you run an interactive program like vim tumx etc.
 *  KeyPad: If this mode is set, the key combination uses a key on the Keypad (Number Pad),This mode is useful to distinguish between keys on the keyboard and keys on the Keypad
 *  AppCursorKeys: DECCKM.
 *  AppKeyPad: DECKPAM
 *
 *
 *
 *
 **/



class QTERM_EXPORT QKxKeyTranslator : public QObject
{
    Q_OBJECT
public:
    enum EOperation {
        ENotDefined = 0,
        ECopy,
        EPaste,
        ESelectLineUp,
        ESelectLineDown,
        ESelectLineLeft,
        ESelectLineRight,
        ESelectLineHome,
        ESelectLineEnd,
        ESelectAll,
        EScrollLineUp,
        EScrollPageUp,
        EScrollUpToTop,
        EScrollLineDown,
        EScrollPageDown,
        EScrollDownToBottom,
        EFind
    };

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
    Q_ENUM(EOperation)

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
    
    struct KeyInfo {
        QString  condition;
        QString  action;

        ModifyKeys  modifies;
        VTModes     modes;
        int         key;

        // operation or key.
        QByteArray  out;
        EOperation  op;

        KeyInfo() {
            op = ENotDefined;
            modes = 0;
            modifies = 0;
        }

        friend bool operator==(const KeyInfo& ki, const KeyInfo& hit) {
            return ki.action == hit.action
                    && ki.condition == hit.condition
                    && ki.modifies == hit.modifies
                    && ki.modes == hit.modes
                    && ki.key == hit.key
                    && ki.out == hit.out
                    && ki.op == hit.op;
        }
    };
public:
    explicit QKxKeyTranslator(QObject *parent = nullptr);
    QString name() const;
    bool load(const QString& file);
    bool match(int key, Qt::KeyboardModifiers modifiers, VTModes modes, EOperation &op, QByteArray &out);

    QKeySequence shortcut(EOperation op, VTModes modes=0);

    static QString operationToName(EOperation op);
    static EOperation operationFromName(const QString& name);
    static QByteArray stringToAnsiSequence(const QString& code);
    static QMap<EOperation, QString> operationDescriptions();
    static QString removeComment(const QString& line);
    static void parseOperation(const QString& key, const QString& code, QList<KeyInfo>& all);
    static void parseString(const QString&key, const QString& code, QList<KeyInfo>& all);
    static bool parseKey(const QString&key, KeyInfo &ki);
    static QStringList tokenArray(const QString& keys);
    static bool modifierMatch(ModifyKeys mk, Qt::KeyboardModifiers modifier);

    static bool load(const QString& file, QList<KeyInfo>& all);
    static bool save(const QString& file, const QString& fileTemplate, const QList<KeyInfo>& all);
    static int maxMaxConditionLength(const QList<KeyInfo>& all);
private:
    QString m_name;
    QMap<int, QList<KeyInfo>> m_keys;

};

#endif // QKEYTRANSLATOR_H
