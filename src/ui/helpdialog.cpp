#include "HelpDialog.h"
#include "ui/ImageButton.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent), m_currentIndex(0)
{
    setWindowIcon(QIcon(":/assets/images/help_icon.png"));
    setWindowTitle("帮助");
    setFixedSize(800, 450); // 您可以根据图片大小调整窗口尺寸

    loadHelpImages();
    setupUI();
    updateUI(); // 初始化UI状态
}

void HelpDialog::loadHelpImages()
{
    // 请确保您已准备好 4 张帮助图片，并已添加到资源文件中
    for (int i = 1; i <= 4; ++i) {
        QPixmap image(QString(":/assets/images/help/help_%1.png").arg(i));
        if (image.isNull()) {
            qWarning() << "Failed to load help image:" << i;
            // 如果加载失败，创建一个占位符
            image = QPixmap(700, 400);
            image.fill(Qt::gray);
        }
        m_helpImages.append(image);
    }
}

void HelpDialog::setupUI()
{
    // 创建控件
    m_leftArrow = new ImageButton(":/assets/images/arrow_left.png", this);
    m_rightArrow = new ImageButton(":/assets/images/arrow_right.png", this);
    m_imageLabel = new QLabel(this);
    m_imageLabel->setScaledContents(true); // 让图片自适应QLabel大小

    // 布局
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_leftArrow);
    layout->addSpacing(10);
    layout->addWidget(m_imageLabel, 1); // 第二个参数1表示图片区域会优先拉伸
    layout->addSpacing(10);
    layout->addWidget(m_rightArrow);
    setLayout(layout);

    // 连接信号和槽
    connect(m_leftArrow, &QPushButton::clicked, this, &HelpDialog::showPreviousImage);
    connect(m_rightArrow, &QPushButton::clicked, this, &HelpDialog::showNextImage);
}

void HelpDialog::showPreviousImage()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        updateUI();
    }
}

void HelpDialog::showNextImage()
{
    if (m_currentIndex < m_helpImages.size() - 1) {
        m_currentIndex++;
        updateUI();
    }
}

// 核心函数：更新图片和箭头状态
void HelpDialog::updateUI()
{
    if (m_helpImages.isEmpty()) return;

    // 1. 更新显示的图片
    m_imageLabel->setPixmap(m_helpImages[m_currentIndex]);

    // 2. 根据当前索引，更新箭头的可用状态
    m_leftArrow->setEnabled(m_currentIndex > 0);
    m_rightArrow->setEnabled(m_currentIndex < m_helpImages.size() - 1);
}
