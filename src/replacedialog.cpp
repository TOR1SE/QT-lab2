#include "replacedialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("替换"));
    setWindowIcon(QIcon(":/icons/replace.svg"));

    m_searchEdit = new QLineEdit(this);
    m_replaceEdit = new QLineEdit(this);

    m_caseCheck = new QCheckBox(tr("区分大小写"), this);

    auto *dirGroup = new QGroupBox(tr("方向"), this);
    m_upRadio = new QRadioButton(tr("向上(&U)"), dirGroup);
    m_downRadio = new QRadioButton(tr("向下(&D)"), dirGroup);
    m_downRadio->setChecked(true);
    auto *dirLayout = new QVBoxLayout(dirGroup);
    dirLayout->addWidget(m_upRadio);
    dirLayout->addWidget(m_downRadio);

    m_findButton = new QPushButton(tr("查找下一个(&F)"), this);
    m_findButton->setDefault(true);
    m_replaceButton = new QPushButton(tr("替换(&R)"), this);
    m_replaceAllButton = new QPushButton(tr("全部替换(&A)"), this);
    m_closeButton = new QPushButton(tr("取消"), this);

    auto *label1 = new QLabel(tr("查找目标:"), this);
    auto *label2 = new QLabel(tr("替换为:"), this);

    auto *grid = new QGridLayout(this);
    grid->addWidget(label1, 0, 0);
    grid->addWidget(m_searchEdit, 0, 1);
    grid->addWidget(m_findButton, 0, 2);
    grid->addWidget(label2, 1, 0);
    grid->addWidget(m_replaceEdit, 1, 1);
    grid->addWidget(m_replaceButton, 1, 2);
    grid->addWidget(m_caseCheck, 2, 0);
    grid->addWidget(dirGroup, 2, 1);
    grid->addWidget(m_replaceAllButton, 2, 2);
    grid->addWidget(m_closeButton, 3, 2);

    auto enableButtons = [this](const QString &t) {
        const bool has = !t.isEmpty();
        m_findButton->setEnabled(has);
        m_replaceButton->setEnabled(has);
        m_replaceAllButton->setEnabled(has);
    };
    connect(m_searchEdit, &QLineEdit::textChanged, this, enableButtons);

    connect(m_findButton, &QPushButton::clicked, this, &ReplaceDialog::onFindNextClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ReplaceDialog::onFindNextClicked);
    connect(m_replaceButton, &QPushButton::clicked, this, &ReplaceDialog::onReplaceOneClicked);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &ReplaceDialog::onReplaceAllClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void ReplaceDialog::setSearchText(const QString &text)
{
    m_searchEdit->setText(text);
    m_searchEdit->selectAll();
    m_searchEdit->setFocus();
}

void ReplaceDialog::onFindNextClicked()
{
    const QString s = m_searchEdit->text();
    if (s.isEmpty())
        return;
    emit findNextRequested(s, m_upRadio->isChecked(), m_caseCheck->isChecked());
}

void ReplaceDialog::onReplaceOneClicked()
{
    const QString s = m_searchEdit->text();
    if (s.isEmpty())
        return;
    emit replaceOneRequested(s, m_replaceEdit->text(), m_upRadio->isChecked(),
                             m_caseCheck->isChecked());
}

void ReplaceDialog::onReplaceAllClicked()
{
    const QString s = m_searchEdit->text();
    if (s.isEmpty())
        return;
    emit replaceAllRequested(s, m_replaceEdit->text(), m_caseCheck->isChecked());
}
