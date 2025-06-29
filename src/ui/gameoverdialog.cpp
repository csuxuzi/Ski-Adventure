#include "ui/GameOverDialog.h"
#include "ui/ImageButton.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>

GameOverDialog::GameOverDialog(QWidget *parent)
    : QDialog(parent)
{
    // 设置为无边框窗口，使其看起来更像游戏内的UI
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();

    // 连接按钮点击信号到我们自定义的信号
    connect(m_restartButton, &QPushButton::clicked, this, &GameOverDialog::restartClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &GameOverDialog::exitClicked);
}

void GameOverDialog::setupUI()
{
    // 1. 创建 "游戏失败" 的文本标签
    QLabel* titleLabel = new QLabel("游戏失败", this);
    titleLabel->setStyleSheet("font-size: 48px; color: white; font-weight: bold;");

    // 2. 创建按钮 (这里我们复用暂停菜单的按钮图片作为示例)
    m_restartButton = new ImageButton(":/assets/images/btn_restart.png", this);
    m_exitButton = new ImageButton(":/assets/images/btn_exit.png", this);

    // 3. 创建水平布局来放置按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->addStretch(); // 添加弹性空间
    buttonLayout->addWidget(m_restartButton);
    buttonLayout->addWidget(m_exitButton);
    buttonLayout->addStretch(); // 添加弹性空间

    // 4. 创建垂直主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->addStretch();
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter); // 标题居中
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}
