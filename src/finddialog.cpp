#include "finddialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("查找"));
    setWindowIcon(QIcon(":/icons/find.svg"));

    m_searchEdit = new QLineEdit(this);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, [this](const QString &t) { m_findButton->setEnabled(!t.isEmpty()); });

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
    m_findButton->setEnabled(false);
    m_closeButton = new QPushButton(tr("取消"), this);

    auto *label = new QLabel(tr("查找目标:"), this);

    auto *grid = new QGridLayout(this);
    grid->addWidget(label, 0, 0);
    grid->addWidget(m_searchEdit, 0, 1);
    grid->addWidget(m_findButton, 0, 2);
    grid->addWidget(m_caseCheck, 1, 0);
    grid->addWidget(dirGroup, 1, 1);
    grid->addWidget(m_closeButton, 1, 2);

    connect(m_findButton, &QPushButton::clicked, this, &FindDialog::onFindClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &FindDialog::onFindClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void FindDialog::setSearchText(const QString &text)
{
    m_searchEdit->setText(text);
    m_searchEdit->selectAll();
    m_searchEdit->setFocus();
}

void FindDialog::onFindClicked()
{
    const QString text = m_searchEdit->text();
    if (text.isEmpty())
        return;
    emit findRequested(text, m_upRadio->isChecked(), m_caseCheck->isChecked());
}
