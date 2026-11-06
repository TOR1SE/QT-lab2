#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCursor>

class CodeEditor;
class FindDialog;
class ReplaceDialog;
class QLabel;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void exitApp();

    // 编辑
    void findText();
    void replaceText();

    // 格式
    void setFontFamily();
    void setFontColor();
    void setFontBgColor();
    void setEditorBgColor();
    void toggleWrap(bool on);
    void toggleLineNumbers(bool on);

    // 查看
    void toggleToolbar(bool on);
    void toggleStatusbar(bool on);

    // 帮助
    void showAbout();

    // 内部
    void onDocumentModified();
    void onCursorPositionChanged();
    void onTextChanged();
    void loadFile(const QString &fileName);
    void onFindRequested(const QString &text, bool backward, bool caseSensitive);
    void onReplaceOneRequested(const QString &searchText, const QString &replaceText,
                               bool backward, bool caseSensitive);
    void onReplaceAllRequested(const QString &searchText, const QString &replaceText,
                               bool caseSensitive);
    void autoSave();

private:
    bool maybeSave();
    bool saveFileTo(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName) const;
    bool findFrom(QTextCursor cursor, const QString &text, bool backward, bool caseSensitive);
    void showStatusBarMessage(const QString &msg, int timeout = 3000);

    Ui::MainWindow *ui;
    CodeEditor *m_editor;
    QString m_currentFile;

    FindDialog *m_findDialog;
    ReplaceDialog *m_replaceDialog;

    QLabel *m_lengthLabel;
    QLabel *m_linesLabel;
    QLabel *m_posLabel;
    QLabel *m_messageLabel;

    QTimer *m_autoSaveTimer;
};

#endif // MAINWINDOW_H
