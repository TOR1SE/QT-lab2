#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

class QPaintEvent;
class QResizeEvent;
class QTextDocument;

class CodeEditor;

// 行号区域：随编辑器滚动同步绘制
class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_codeEditor;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    bool lineNumbersVisible() const { return m_showLineNumbers; }

public slots:
    void setLineNumbersVisible(bool on);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void updateExtraSelections();

private:
    QWidget *m_lineNumberArea;
    bool m_showLineNumbers;
    QColor m_lineHighlightColor;
    QColor m_backgroundColor;
};

#endif // CODEEDITOR_H
