#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QRadioButton;
class QPushButton;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QWidget *parent = nullptr);

    void setSearchText(const QString &text);

signals:
    // backward = true 表示向上查找
    void findRequested(const QString &text, bool backward, bool caseSensitive);

private slots:
    void onFindClicked();

private:
    QLineEdit *m_searchEdit;
    QCheckBox *m_caseCheck;
    QRadioButton *m_upRadio;
    QRadioButton *m_downRadio;
    QPushButton *m_findButton;
    QPushButton *m_closeButton;
};

#endif // FINDDIALOG_H
