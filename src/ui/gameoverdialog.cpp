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

void GameOverDialog::setScore(quint64 score)
{
    if (m_scoreLabel) {
        m_scoreLabel->setText(QString("本次得分: %1").arg(score));
    }
}

void GameOverDialog::setupUI()
{
    // 创建 "游戏失败" 的文本标签
    m_titleLabel = new QLabel("游戏结束", this);
    m_titleLabel->setStyleSheet("font-size: 48px; color: white; font-weight: bold;");

    // 创建分数标签
    m_scoreLabel = new QLabel("本次得分: 0", this);
    m_scoreLabel->setStyleSheet("font-size: 24px; color: white;");
    m_scoreLabel->setAlignment(Qt::AlignCenter);


    // 创建按钮
    m_restartButton = new ImageButton(":/assets/images/btn_restart.png", this);
    m_exitButton = new ImageButton(":/assets/images/btn_exit.png", this);

    // 创建水平布局来放置按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->addStretch(); // 添加弹性空间
    buttonLayout->addWidget(m_restartButton);
    buttonLayout->addWidget(m_exitButton);
    buttonLayout->addStretch(); // 添加弹性空间

    // 创建垂直主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->addStretch();
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter); // 标题居中
    mainLayout->addWidget(m_scoreLabel, 0, Qt::AlignHCenter); // 添加分数标签
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}
