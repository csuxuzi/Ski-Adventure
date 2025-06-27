#include "ui/SettingsDialog.h"
#include "audio/AudioManager.h"
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QApplication>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("设置");
    setFixedSize(300, 200);

    // 创建控件
    QCheckBox* musicCheckbox = new QCheckBox("开启音乐", this);
    musicCheckbox->setChecked(true);
    QCheckBox* sfxCheckbox = new QCheckBox("开启音效", this);
    sfxCheckbox->setChecked(true);
    QPushButton* closeButton = new QPushButton("关闭", this);
    QPushButton* exitButton = new QPushButton("退出游戏", this);


    // 布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(musicCheckbox);
    layout->addWidget(sfxCheckbox);
    layout->addStretch();
    layout->addWidget(closeButton);
    layout->addWidget(exitButton);
    setLayout(layout);

    // 连接信号和槽
    connect(musicCheckbox, &QCheckBox::stateChanged, this, &SettingsDialog::onMusicStateChanged);
    connect(sfxCheckbox, &QCheckBox::stateChanged, this, &SettingsDialog::onSfxStateChanged);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept); // accept()会关闭对话框
    connect(exitButton, &QPushButton::clicked, this, &SettingsDialog::onExitGameClicked);
}

void SettingsDialog::onMusicStateChanged(int state)
{
    bool enabled = (state == Qt::Checked);
    AudioManager::instance()->setMusicEnabled(enabled);
}

void SettingsDialog::onSfxStateChanged(int state)
{
    bool enabled = (state == Qt::Checked);
    AudioManager::instance()->setSfxEnabled(enabled);
}

void SettingsDialog::onExitGameClicked()
{
    qApp->quit(); // 退出整个应用程序
}