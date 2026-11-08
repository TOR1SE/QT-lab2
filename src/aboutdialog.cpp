#include "aboutdialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Dialog"));
    setFixedSize(420, 300);

    auto *title = new QLabel(tr("苗苗文本编辑器"), this);
    title->setStyleSheet("font-size: 30px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);

    auto *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/icons/paste.svg").scaled(
        96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    auto *info = new QLabel(this);
    info->setText(tr("开发者：林苗苗<br>"
                     "学号：2024414300214<br>"
                     "班级：24软件工程（卓越计划）2班"));
    info->setAlignment(Qt::AlignVCenter);

    auto *midLayout = new QHBoxLayout;
    midLayout->addWidget(iconLabel);
    midLayout->addSpacing(20);
    midLayout->addWidget(info);
    midLayout->addStretch();

    auto *version = new QLabel(tr("V1.0 build 20261102"), this);

    auto *okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    auto *grid = new QGridLayout(this);
    grid->addWidget(title, 0, 0);
    grid->addLayout(midLayout, 1, 0);
    grid->addWidget(version, 2, 0);
    grid->addWidget(okButton, 3, 0, Qt::AlignRight);

    grid->setContentsMargins(24, 16, 24, 16);
    grid->setVerticalSpacing(14);
}
