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

#ifndef QKXCOMBINEKEYACTIONDIALOG_H
#define QKXCOMBINEKEYACTIONDIALOG_H


#include "qvtedef.h"
#include "qkxkeytranslator.h"

#include <QDialog>
#include <QPointer>

namespace Ui {
class QKxCombineKeyActionDialog;
}

class QTERM_EXPORT QKxCombineKeyActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QKxCombineKeyActionDialog(QWidget *parent = nullptr);
    ~QKxCombineKeyActionDialog();

    void init(const QKxKeyTranslator::KeyInfo& ki);
    QKxKeyTranslator::KeyInfo result();
signals:
    void messageArrived(const QString& title, const QString& msg);
private slots:
    void onActionButtonClicked();
    void onApplyButtonClicked();
    void onCombineKeyChanged();
    void onQuickInput();
private:
    void adjustSizeLater();
    QString combineKey();
    void setShortcut(const QKeySequence &seq);

private:
    virtual bool eventFilter(QObject *watched, QEvent *ev);

private:
    Ui::QKxCombineKeyActionDialog *ui;
};

#endif // QKXCOMBINEKEYACTIONDIALOG_H
