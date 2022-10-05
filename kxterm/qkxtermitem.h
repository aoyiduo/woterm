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

#ifndef QTERM_H
#define QTERM_H

#include "qvtedef.h"

#include <QWidget>
#include <QPointer>
#include <QCursor>

class QVteImpl;
class QKxView;
class QTheme;
class QKxKeyTranslator;
class QKxColorSchema;
class QTimer;
class QLabel;

class QKxEchoInput;
class QTERM_EXPORT QKxTermItem : public QWidget
{
    Q_OBJECT
public:
    enum CursorType {
        CT_Block = 0,
        CT_Underline,
        CT_IBeam
    };
    enum PaintFlag {
        PF_None = 0,
        PF_Normal = 1,
        PF_Selection = 2,
        PF_Scroll = 4,
        PF_FullScreen = 8
    };
    enum ShortCutKey{
        SCK_Copy = 1,
        SCK_Paste,
        SCK_SelectAll,
        SCK_SelectLeft,
        SCK_SelectRight,
        SCK_SelectUp,
        SCK_SelectDown,
        SCK_SelectHome,
        SCK_SelectEnd,
    };
    Q_ENUM(CursorType)
    Q_ENUM(SelectionMode)
    Q_ENUM(ShortCutKey)
    Q_DECLARE_FLAGS(PaintFlags, PaintFlag)

    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily)
    Q_PROPERTY(bool blinkAlway READ blinkAlway WRITE setBlinkAlway)

    Q_PROPERTY(int scrollValue READ scrollValue)
    Q_PROPERTY(int scrollMaxValue READ scrollMaxValue)
    Q_PROPERTY(bool highContrast READ highContrast WRITE setHighContrast)
    Q_PROPERTY(CursorType cursorType READ cursorType WRITE setCursorType)
    Q_PROPERTY(QString keyTable READ keyTable)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(QColor background READ backgroundColor)
    Q_PROPERTY(QString name READ termName WRITE setTermName)
    Q_PROPERTY(int historySize READ historySize WRITE setHistorySize)
    Q_PROPERTY(QString colorSchema READ colorSchema WRITE setColorSchema)
    Q_PROPERTY(bool inputEnable READ inputEnable WRITE setInputEnable)
public:
    explicit QKxTermItem(QWidget *parent = nullptr);

    QFont font() const;
    void setFont(const QFont& ft);

    QString textCodec() const;
    void setTextCodec(const QString& c);

    bool inputEnable() const;
    void setInputEnable(bool on);

    int fontSize() const;
    void setFontSize(int sz);

    QString fontFamily() const;
    void setFontFamily(const QString& name);

    bool blinkAlway() const;
    void setBlinkAlway(bool on);

    int scrollMaxValue() const;
    int scrollValue() const;

    bool highContrast() const;
    void setHighContrast(bool on);

    void setBackgroundColor(const QColor& clr);
    QColor backgroundColor() const;

    CursorType cursorType() const;
    void setCursorType(const CursorType& t);

    QString keyTable() const;

    QCursor mouseCursor() const;
    void setMouseCursor(const QCursor& cur);

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode m);

    QSize termSize() const;
    QSize termViewSize() const;
    QString selectedText() const;

    bool echoInputEnabled() const;
    void setEchoInputEnabled(bool on);
    void setEchoInput(QKxEchoInput *echo);

    void scrollToEnd();
    bool scrollTo(int y);

    QKxKeyTranslator *keyLayout() const;
    void setKeyLayout(QKxKeyTranslator *translator);
    QString keyLayoutName() const;
    void setKeyLayoutByName(const QString& name);

    QString colorSchema() const;
    void setColorSchema(const QString& name);

    QString termName() const;
    void setTermName(const QString& name);

    QPoint cursorToScreenPosition();
    QPoint cursorToViewPosition();
    QPoint viewToScreenPosition(const QPoint& pt);

    int historySize() const;
    void setHistorySize(int s);
    void setHistoryFile(const QString& file);
    void stopHistoryFile();
    void cleanHistory();

    bool appMode();
    QString plainText(const QPoint& start, const QPoint& end);

    // trapCommand
    bool trapCommand(const QString& cmd, QString& content, int &code, int timeout=3000);
    void waitInput();

    // shortcut
    void bindShortCut(ShortCutKey sck, QKeySequence key);
    QKeySequence defaultShortCutKey(ShortCutKey sck);
    QKeySequence currentShortCutKey(ShortCutKey sck);
    // theme
    Q_INVOKABLE QStringList availableKeyLayouts() const;
    Q_INVOKABLE QStringList availableColorSchemas() const;
    Q_INVOKABLE QStringList availableFontFamilies() const;
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void preview();
    // copy or paste.
    Q_INVOKABLE void tryToCopy();
    Q_INVOKABLE void tryToPaste();
signals:
    void sendData(const QByteArray& buf);
    void termSizeChanged(int lines, int columns);
    //void scrollValueChanged(qreal lines, qreal position);
    void scrollValueChanged(int lines, int position);
    void backgroundChanged(const QColor &clr);
    void titleChanged(const QString& title);

public slots:
    void onScreenChanged();
    void onBlinkTimeout();
    void onSelectChanged();
    void onContentChanged(bool full);
    void onRepaintTimeout();
    void onTripleClickTimeout();
    void onSelectClickTimeout();
public:
    Q_INVOKABLE void resetState();
    Q_INVOKABLE void echoInput(const QByteArray& data);
    Q_INVOKABLE void parse(const QByteArray& data);
    Q_INVOKABLE void parseError(const QByteArray& data);
    Q_INVOKABLE void updateTermSize(int rows, int cols);
    Q_INVOKABLE void updateScrollPosition(qreal position);
    Q_INVOKABLE bool loadKeyLayout(const QString &path);
    Q_INVOKABLE bool loadColorSchema(const QString &path);
    Q_INVOKABLE bool find(const QString& key, bool match, bool regular);
    Q_INVOKABLE bool findPrev(bool match, bool regular);
    Q_INVOKABLE bool findNext(bool match, bool regular);
    Q_INVOKABLE void findAll(bool match, bool regular);
    Q_INVOKABLE void clearSelection();    
    Q_INVOKABLE void showInputMethod(bool show);
protected:
    virtual void paint(QPainter *p);
    virtual void paintEvent(QPaintEvent *ev);
    virtual void focusInEvent(QFocusEvent *ev);
    virtual void focusOutEvent(QFocusEvent *ev);
    virtual void resizeEvent(QResizeEvent *ev);
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void keyReleaseEvent(QKeyEvent *ev);
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent* ev);
    virtual void wheelEvent(QWheelEvent *ev);
    virtual bool focusNextPrevChild(bool next);

    virtual void inputMethodEvent(QInputMethodEvent *ev);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;    
    virtual bool event(QEvent *e);
    virtual bool eventFilter(QObject *watched, QEvent *ev);
private:
    QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;
    void updateView(PaintFlag flag = PF_Normal);
    void updateScrollValue(int v);
    void updateFontInfo();
    void updateTermSize();
    void updateImage();
    void drawLine(QPainter *p, int r, const TermLine& line);
    void drawCursor(QPainter *p, const QRect& rt, const QColor& bg, const QColor& fg,  bool &inverse);
    void handleKeyEvent(QKeyEvent *ev);
    void scroll(int offsety);
    bool isLineVisible(int y);
    // -1: timeout.
    //  1: title changed.
    int wait(int timeout=3000);
    int lastExitCode();
    void initTitle();
    void resetTitlePosition(bool byCursor = false);
private:
    QPointer<QLabel> m_title;
    QPointer<QVteImpl> m_vte;
    QPointer<QKxView> m_view;
    QList<TermLine> m_image;
    QPointer<QKxKeyTranslator> m_keyTranslator;
    QPointer<QKxColorSchema> m_colorSchema;
    //blink
    QPointer<QTimer> m_blinker;
    QList<QRect> m_blinkRects;
    int m_blinkCountLeft;
    bool m_blinkShow;

    //repaint timer
    QPointer<QTimer> m_ticker;
    QPointer<QTimer> m_ticker2;
    PaintFlags m_flagPaints;

    int m_rows;
    int m_columns;
    QFont m_font;
    int m_spaceLine;
    int m_fontWidth;
    int m_fontHeight;
    int m_lineWidth;
    int m_scrollValue;
    int m_scrollMaxValue;
    bool m_autoScrollToEnd;
    bool m_highContrast;

    //cursor
    bool m_cursorReverse;
    int m_cursorLeftBlink;
    int m_cursorLeftReverse;
    bool m_screenReverse;
    CursorType m_cursorType;
    QRect m_cursorRect;
    QRect m_cursorRectLast;

    // focus
    bool m_focus;

    // selection.
    QPoint m_selectStart;
    QPoint m_selectEnd;
    SelectionMode m_selectMode;
    bool m_tripleClick;

    QString m_schema;

    QString m_preeditText;
    QRect m_preeditRect;

    int m_touchCount;

    bool m_blinkAlway;

    QPointF m_touch;

    QColor m_backgroundColor;

    QKeySequence m_keyCopy;
    QKeySequence m_keyPaste;
    QKeySequence m_keyUpSelect, m_keyDownSelect, m_keyLeftSelect, m_keyRightSelect;
    QKeySequence m_keyHomeSelect, m_keyEndSelect, m_keySelectAll;
    QPoint m_ptClicked;

    bool m_bEchoInputEnabled;
    QPointer<QKxEchoInput> m_echoInput;
};

#endif // QTERM_H
