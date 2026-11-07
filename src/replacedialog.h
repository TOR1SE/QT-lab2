#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QRadioButton;
class QPushButton;

class ReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReplaceDialog(QWidget *parent = nullptr);

    void setSearchText(const QString &text);

signals:
    void findNextRequested(const QString &searchText, bool backward, bool caseSensitive);
    void replaceOneRequested(const QString &searchText, const QString &replaceText,
                             bool backward, bool caseSensitive);
    void replaceAllRequested(const QString &searchText, const QString &replaceText,
                             bool caseSensitive);

private slots:
    void onFindNextClicked();
    void onReplaceOneClicked();
    void onReplaceAllClicked();

private:
    QLineEdit *m_searchEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseCheck;
    QRadioButton *m_upRadio;
    QRadioButton *m_downRadio;
    QPushButton *m_findButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceAllButton;
    QPushButton *m_closeButton;
};

#endif // REPLACEDIALOG_H
