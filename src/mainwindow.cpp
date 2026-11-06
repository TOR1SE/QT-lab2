#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QCloseEvent>
#include <QAbstractButton>
#include <QColorDialog>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

namespace {
const char *kAppTitle = QT_TRANSLATE_NOOP("MainWindow", "编辑器");
const char *kUntitled = "新建文本文件";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_editor(nullptr),
      m_findDialog(nullptr),
      m_replaceDialog(nullptr)
{
    ui->setupUi(this);
    m_editor = ui->editor;

    // ---------- 状态栏 ----------
    m_lengthLabel = new QLabel(this);
    m_linesLabel = new QLabel(this);
    m_posLabel = new QLabel(this);
    m_messageLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_lengthLabel);
    statusBar()->addPermanentWidget(m_linesLabel);
    statusBar()->addPermanentWidget(m_posLabel);
    // 与参考图一致：length / lines / Ln / Col
    m_lengthLabel->setMinimumWidth(90);
    m_linesLabel->setMinimumWidth(90);
    m_posLabel->setMinimumWidth(110);

    // ---------- Action 连接 ----------
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApp);

    connect(ui->actionUndo, &QAction::triggered, m_editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, m_editor, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, m_editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, m_editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, m_editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, m_editor, &QPlainTextEdit::selectAll);
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::findText);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::replaceText);

    // 编辑状态与 Action 使能同步
    connect(m_editor->document(), &QTextDocument::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(m_editor->document(), &QTextDocument::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(m_editor, &QPlainTextEdit::copyAvailable, ui->actionCopy, &QAction::setEnabled);
    ui->actionPaste->setEnabled(true);

    // 格式
    connect(ui->actionWrap, &QAction::toggled, this, &MainWindow::toggleWrap);
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumbers);
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::setFontFamily);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::setFontColor);
    connect(ui->actionFontBgColor, &QAction::triggered, this, &MainWindow::setFontBgColor);
    connect(ui->actionEditorBgColor, &QAction::triggered, this, &MainWindow::setEditorBgColor);

    // 查看
    ui->actionToolbar->setChecked(true);
    ui->actionStatusbar->setChecked(true);
    connect(ui->actionToolbar, &QAction::toggled, this, &MainWindow::toggleToolbar);
    connect(ui->actionStatusbar, &QAction::toggled, this, &MainWindow::toggleStatusbar);

    // 帮助
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);

    // ---------- 文档状态 → 标题 / 状态栏 ----------
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::onDocumentModified);
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::onCursorPositionChanged);
    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);

    // ---------- 查找 / 替换对话框 ----------
    m_findDialog = new FindDialog(this);
    m_replaceDialog = new ReplaceDialog(this);
    connect(m_findDialog, &FindDialog::findRequested,
            this, &MainWindow::onFindRequested);
    connect(m_replaceDialog, &ReplaceDialog::findNextRequested,
            this, &MainWindow::onFindRequested);
    connect(m_replaceDialog, &ReplaceDialog::replaceOneRequested,
            this, &MainWindow::onReplaceOneRequested);
    connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested,
            this, &MainWindow::onReplaceAllRequested);

    // ---------- 定时自动保存（选做） ----------
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(60 * 1000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    m_editor->setFocus();
    onTextChanged();
    onCursorPositionChanged();
    setCurrentFile(QString());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------- 文件 ----------------

void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    m_editor->clear();
    m_editor->document()->setModified(false);
    setCurrentFile(QString());
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QString(),
        tr("文本文件 (*.txt *.h *.cpp *.c *.md *.log);;所有文件 (*)"),
        &selectedFilter, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty())
        return saveFileAs();
    return saveFileTo(m_currentFile);
}

bool MainWindow::saveFileAs()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("另存为"), QString(),
        tr("文本文件 (*.txt);;所有文件 (*)"),
        &selectedFilter, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return false;
    return saveFileTo(fileName);
}

void MainWindow::exitApp()
{
    close();
}

bool MainWindow::saveFileTo(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("编辑器"),
                             tr("无法保存文件 %1:\n%2.").arg(QDir::toNativeSeparators(fileName),
                                                            file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_editor->toPlainText();
    file.close();

    setCurrentFile(fileName);
    showStatusBarMessage(tr("已保存: %1").arg(strippedName(fileName)));
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        QMessageBox::warning(this, tr("编辑器"),
                             tr("文件不存在: %1").arg(QDir::toNativeSeparators(fileName)));
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("编辑器"),
                             tr("无法打开文件 %1:\n%2.").arg(QDir::toNativeSeparators(fileName),
                                                            file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_editor->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();
    file.close();

    setCurrentFile(fileName);
    showStatusBarMessage(tr("已打开: %1").arg(strippedName(fileName)));
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    m_editor->document()->setModified(false);

    QString shownName;
    if (!m_currentFile.isEmpty()) {
        shownName = strippedName(m_currentFile);
    } else {
        shownName = tr(kUntitled);
    }
    // [*] 占位符由 setWindowModified 控制
    setWindowTitle(tr("%1[*] - %2").arg(shownName, tr(kAppTitle)));
}

QString MainWindow::strippedName(const QString &fullFileName) const
{
    return QFileInfo(fullFileName).fileName();
}

// 未保存时询问是否保存；返回 false 表示用户取消操作
bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified())
        return true;

    QMessageBox box(QMessageBox::Warning, tr("编辑器"),
                    tr("文档已被修改。\n是否保存更改?"),
                    QMessageBox::NoButton, this);
    QPushButton *saveButton = box.addButton(tr("保存"), QMessageBox::AcceptRole);
    box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    QPushButton *cancelButton = box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.exec();

    const QAbstractButton *clicked = box.clickedButton();
    if (clicked == static_cast<QAbstractButton *>(saveButton))
        return saveFile();
    if (clicked == static_cast<QAbstractButton *>(cancelButton) || clicked == nullptr)
        return false;
    return true;   // 不保存
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ---------------- 编辑 ----------------

void MainWindow::findText()
{
    // 把当前选中文字预填进查找框
    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QLatin1Char('\n')))
        m_findDialog->setSearchText(selected);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::replaceText()
{
    const QString selected = m_editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QLatin1Char('\n')))
        m_replaceDialog->setSearchText(selected);
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

// 从 cursor 位置查找；找不到时自动从文档另一端循环查找一次
bool MainWindow::findFrom(QTextCursor cursor, const QString &text,
                          bool backward, bool caseSensitive)
{
    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *doc = m_editor->document();
    QTextCursor found = doc->find(text, cursor, flags);
    if (found.isNull()) {
        // 循环查找：向下查不到就回到文档开头，向上查不到就回到文档末尾
        QTextCursor wrapCursor = m_editor->textCursor();
        if (backward) {
            wrapCursor.movePosition(QTextCursor::End);
            found = doc->find(text, wrapCursor, flags);
        } else {
            wrapCursor.movePosition(QTextCursor::Start);
            found = doc->find(text, wrapCursor, flags);
        }
    }

    if (found.isNull()) {
        QMessageBox::information(this, tr("查找"),
                                 tr("找不到 \"%1\"").arg(text));
        return false;
    }

    m_editor->setTextCursor(found);
    return true;
}

void MainWindow::onFindRequested(const QString &text, bool backward, bool caseSensitive)
{
    findFrom(m_editor->textCursor(), text, backward, caseSensitive);
}

void MainWindow::onReplaceOneRequested(const QString &searchText, const QString &replaceText,
                                       bool backward, bool caseSensitive)
{
    // 若当前选中的内容正好匹配，先替换；否则先定位
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection()
        || cursor.selectedText().compare(searchText,
                                         caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive) != 0) {
        findFrom(cursor, searchText, backward, caseSensitive);
        return;
    }

    const bool wasModified = m_editor->document()->isModified();
    cursor.insertText(replaceText);
    m_editor->document()->setModified(wasModified || true);
    showStatusBarMessage(tr("已替换 1 处"));
}

void MainWindow::onReplaceAllRequested(const QString &searchText, const QString &replaceText,
                                       bool caseSensitive)
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *doc = m_editor->document();
    int count = 0;
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    while (true) {
        cursor = doc->find(searchText, cursor, flags);
        if (cursor.isNull())
            break;
        cursor.insertText(replaceText);
        ++count;
    }
    cursor.endEditBlock();

    QMessageBox::information(this, tr("替换"),
                             tr("共替换 %1 处.").arg(count));
}

// ---------------- 格式 ----------------

void MainWindow::setFontFamily()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, m_editor->font(), this, tr("选择字体"));
    if (!ok)
        return;

    // 有选中文字时只设置选中部分，否则设置整个编辑器
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        QTextCharFormat format;
        format.setFont(font);
        cursor.mergeCharFormat(format);
        m_editor->mergeCurrentCharFormat(format);
    } else {
        m_editor->setFont(font);
    }
}

void MainWindow::setFontColor()
{
    QColor color = QColorDialog::getColor(Qt::black, this, tr("字体颜色"));
    if (!color.isValid())
        return;

    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat format;
    format.setForeground(color);
    if (cursor.hasSelection())
        cursor.mergeCharFormat(format);
    else
        m_editor->mergeCurrentCharFormat(format);
}

void MainWindow::setFontBgColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("字体背景色"));
    if (!color.isValid())
        return;

    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat format;
    format.setBackground(color);
    if (cursor.hasSelection())
        cursor.mergeCharFormat(format);
    else
        m_editor->mergeCurrentCharFormat(format);
}

void MainWindow::setEditorBgColor()
{
    QColor color = QColorDialog::getColor(m_editor->palette().color(QPalette::Base),
                                          this, tr("编辑器背景色"));
    if (!color.isValid())
        return;

    QPalette palette = m_editor->palette();
    palette.setColor(QPalette::Base, color);
    palette.setColor(QPalette::Window, color);
    m_editor->setPalette(palette);
}

void MainWindow::toggleWrap(bool on)
{
    m_editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                 : QPlainTextEdit::NoWrap);
}

void MainWindow::toggleLineNumbers(bool on)
{
    m_editor->setLineNumbersVisible(on);
}

// ---------------- 查看 ----------------

void MainWindow::toggleToolbar(bool on)
{
    ui->toolBar->setVisible(on);
}

void MainWindow::toggleStatusbar(bool on)
{
    statusBar()->setVisible(on);
}

// ---------------- 帮助 ----------------

void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ---------------- 状态更新 ----------------

void MainWindow::onDocumentModified()
{
    setWindowModified(m_editor->document()->isModified());
}

void MainWindow::onTextChanged()
{
    m_lengthLabel->setText(tr("length: %1").arg(m_editor->toPlainText().length()));
    m_linesLabel->setText(tr("lines: %1").arg(m_editor->document()->blockCount()));
}

void MainWindow::onCursorPositionChanged()
{
    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    m_posLabel->setText(tr("Ln: %1  Col: %2").arg(line).arg(column));
}

void MainWindow::showStatusBarMessage(const QString &msg, int timeout)
{
    statusBar()->showMessage(msg, timeout);
}

void MainWindow::autoSave()
{
    // 自动保存：只针对已经存过盘的文件，把当前内容写入同名 .autosave 副本
    if (m_currentFile.isEmpty() || !m_editor->document()->isModified())
        return;

    QFile file(m_currentFile + QLatin1String(".autosave"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_editor->toPlainText();
    file.close();

    showStatusBarMessage(tr("已自动保存到 %1.autosave").arg(strippedName(m_currentFile)));
}
