#include "ui/SettingsDialog.h"
#include "ui/ImageButton.h"
#include "audio/AudioManager.h"
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("设置");
    setFixedSize(450, 250);
    setupUI();

    // 连接所有信号和槽
    connect(m_musicIcon, &QPushButton::clicked, this, &SettingsDialog::onMusicIconClicked);
    connect(m_sfxIcon, &QPushButton::clicked, this, &SettingsDialog::onSfxIconClicked);
    connect(m_musicSlider, &QSlider::valueChanged, this, &SettingsDialog::onMusicVolumeChanged);
    connect(m_sfxSlider, &QSlider::valueChanged, this, &SettingsDialog::onSfxVolumeChanged);
}

// 每次显示对话框时，都从AudioManager重新加载状态
void SettingsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    // 断开连接，防止setValue触发valueChanged信号
    m_musicSlider->blockSignals(true);
    m_sfxSlider->blockSignals(true);

    m_musicSlider->setValue(AudioManager::instance()->getMusicVolume() * 100);
    m_sfxSlider->setValue(AudioManager::instance()->getSfxVolume() * 100);
    updateMusicIcon();
    updateSfxIcon();

    // 恢复连接
    m_musicSlider->blockSignals(false);
    m_sfxSlider->blockSignals(false);
}

void SettingsDialog::setupUI()
{
    m_musicIcon = new ImageButton(":/assets/images/icon_music_on.png", this);
    m_sfxIcon = new ImageButton(":/assets/images/icon_sfx_on.png", this);

    m_musicSlider = new QSlider(Qt::Horizontal, this);
    m_sfxSlider = new QSlider(Qt::Horizontal, this);
    m_musicSlider->setRange(0, 100);
    m_sfxSlider->setRange(0, 100);

    QLabel* musicLabel = new QLabel("背景音乐", this);
    QLabel* sfxLabel = new QLabel("音效", this);
    musicLabel->setStyleSheet("font-size: 16px;");
    sfxLabel->setStyleSheet("font-size: 16px;");

    // QPushButton* closeButton = new QPushButton("关闭", this);
    // connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* musicLayout = new QHBoxLayout();
    musicLayout->addWidget(m_musicIcon);
    musicLayout->addWidget(musicLabel);
    musicLayout->addStretch();
    musicLayout->addWidget(m_musicSlider);

    QHBoxLayout* sfxLayout = new QHBoxLayout();
    sfxLayout->addWidget(m_sfxIcon);
    sfxLayout->addWidget(sfxLabel);
    sfxLayout->addStretch();
    sfxLayout->addWidget(m_sfxSlider);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(musicLayout);
    mainLayout->addLayout(sfxLayout);
    mainLayout->addStretch();
    //mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);
    setLayout(mainLayout);
}

void SettingsDialog::onMusicIconClicked()
{
    // 点击图标时，只在0和70之间切换
    if (m_musicSlider->value() > 0) {
        m_musicSlider->setValue(0);
    } else {
        m_musicSlider->setValue(70);
    }
}

void SettingsDialog::onSfxIconClicked()
{
    if (m_sfxSlider->value() > 0) {
        m_sfxSlider->setValue(0);
    } else {
        m_sfxSlider->setValue(70);
    }
}

void SettingsDialog::onMusicVolumeChanged(int value)
{
    AudioManager::instance()->setMusicVolume(value / 100.0f);
    updateMusicIcon();
}

void SettingsDialog::onSfxVolumeChanged(int value)
{
    AudioManager::instance()->setSfxVolume(value / 100.0f);
    updateSfxIcon();
}

void SettingsDialog::updateMusicIcon()
{
    // 图标状态只取决于音量是否为0
    if (m_musicSlider->value() > 0) {
        m_musicIcon->updateIcon(QPixmap(":/assets/images/icon_music_on.png"));
        AudioManager::instance()->setMusicEnabled(true);
    } else {
        m_musicIcon->updateIcon(QPixmap(":/assets/images/icon_music_off.png"));
        AudioManager::instance()->setMusicEnabled(false);
    }
}

void SettingsDialog::updateSfxIcon()
{
    if (m_sfxSlider->value() > 0) {
        m_sfxIcon->updateIcon(QPixmap(":/assets/images/icon_sfx_on.png"));
        AudioManager::instance()->setSfxEnabled(true);
    } else {
        m_sfxIcon->updateIcon(QPixmap(":/assets/images/icon_sfx_off.png"));
        AudioManager::instance()->setSfxEnabled(false);
    }
}
