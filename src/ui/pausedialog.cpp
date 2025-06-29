#include "ui/PauseDialog.h"
#include "ui/ImageButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

PauseDialog::PauseDialog(QWidget *parent)
    : QDialog(parent)
{
    // 设置对话框为无边框窗口，使其看起来更像游戏内的UI
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    // 设置背景半透明
    setAttribute(Qt::WA_TranslucentBackground);

    setupUI();

    // 连接按钮的点击信号到我们自定义的信号上
    connect(m_resumeButton, &QPushButton::clicked, this, &PauseDialog::resumeClicked);
    connect(m_restartButton, &QPushButton::clicked, this, &PauseDialog::restartClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &PauseDialog::exitClicked);

    // 按下 "继续游戏" 按钮时，也关闭对话框
    connect(m_resumeButton, &QPushButton::clicked, this, &QDialog::accept);
    // 【新增】按下“退出游戏”按钮时，也关闭对话框
    connect(m_exitButton, &QPushButton::clicked, this, &QDialog::reject);
}

void PauseDialog::setupUI()
{
    // 假设您已经准备好了以下图片资源
    m_resumeButton = new ImageButton(":/assets/images/btn_resume.png", this);
    m_restartButton = new ImageButton(":/assets/images/btn_restart.png", this);
    m_exitButton = new ImageButton(":/assets/images/btn_exit.png", this);

    // 使用水平布局来排列这三个按钮
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(20); // 设置按钮之间的间距
    layout->addWidget(m_restartButton);
    layout->addWidget(m_resumeButton); // 把 "继续" 放在中间
    layout->addWidget(m_exitButton);

    setLayout(layout);
}
