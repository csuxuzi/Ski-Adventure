#include "ui/MainMenuScreen.h"
#include "ui/ImageButton.h"
#include "ui/PlaceholderDialog.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>

MainMenuScreen::MainMenuScreen(QWidget *parent) : QWidget(parent)
{
    setupScene();
    setupUI();

    // 将 QGraphicsView 作为此 Widget 的主要布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_view);
    setLayout(mainLayout);

    // 连接开始游戏按钮的信号
    connect(m_startButton, &QPushButton::clicked, this, &MainMenuScreen::startAvalancheAnimation);
}

void MainMenuScreen::setupScene()
{
    m_scene = new QGraphicsScene(0, 0, 800, 600);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setRenderHint(QPainter::Antialiasing);

    // 1. 添加背景
    QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(QPixmap(":/assets/images/scene_background.png"));
    m_scene->addItem(bgItem);

    // 2. 添加房子
    QGraphicsPixmapItem* houseItem = new QGraphicsPixmapItem(QPixmap(":/assets/images/house.png"));
    houseItem->setPos(100, 350); // 您可以调整房子的初始位置
    m_scene->addItem(houseItem);
}

void MainMenuScreen::setupUI()
{
    // --- 1. 创建开始按钮 ---
    m_startButton = new ImageButton(":/assets/images/btn_start.png", this);
    m_startButton->move(this->width() - m_startButton->width() - 20,
                        this->height() - m_startButton->height() - 20);

    // --- 2. 创建抽屉菜单 ---
    // 抽屉的容器
    m_drawerContainer = new QWidget(this);
    m_drawerContainer->setStyleSheet("background-image: url(:/assets/images/drawer_background.png);");

    // 抽屉内的按钮
    ImageButton* helpBtn = new ImageButton(":/assets/images/btn_help.png", m_drawerContainer);
    ImageButton* recordsBtn = new ImageButton(":/assets/images/btn_records.png", m_drawerContainer);
    ImageButton* settingsBtn = new ImageButton(":/assets/images/btn_settings.png", m_drawerContainer);
    ImageButton* highscoreBtn = new ImageButton(":/assets/images/btn_highscore.png", m_drawerContainer);

    // 使用布局管理抽屉内的按钮
    QVBoxLayout* drawerLayout = new QVBoxLayout(m_drawerContainer);
    drawerLayout->setContentsMargins(15, 15, 15, 15);
    drawerLayout->setSpacing(20);
    drawerLayout->addWidget(helpBtn, 0, Qt::AlignHCenter);
    drawerLayout->addWidget(recordsBtn, 0, Qt::AlignHCenter);
    drawerLayout->addWidget(settingsBtn, 0, Qt::AlignHCenter);
    drawerLayout->addWidget(highscoreBtn, 0, Qt::AlignHCenter);
    drawerLayout->addStretch();
    m_drawerContainer->setLayout(drawerLayout);
    m_drawerContainer->setFixedSize(80, 280); // 根据您的背景图调整

    // 控制抽屉打开/关闭的按钮
    m_drawerButton = new ImageButton(":/assets/images/drawer_arrow_open.png", this);
    m_drawerButton->move(10, this->height() - m_drawerButton->height() - 10);
    connect(m_drawerButton, &QPushButton::clicked, this, &MainMenuScreen::toggleDrawer);

    // 将抽屉的初始位置放在屏幕外
    m_drawerContainer->move(10, this->height());

    // --- 3. 连接抽屉内按钮的信号 ---
    connect(settingsBtn, &QPushButton::clicked, this, &MainMenuScreen::settingsClicked);
    connect(helpBtn, &QPushButton::clicked, this, &MainMenuScreen::showHelp);
    connect(recordsBtn, &QPushButton::clicked, this, &MainMenuScreen::showRecords);
    connect(highscoreBtn, &QPushButton::clicked, this, &MainMenuScreen::showHighscore);
}

void MainMenuScreen::toggleDrawer()
{
    int startY, endY;
    if (m_isDrawerOpen) {
        // 关闭抽屉
        startY = this->height() - m_drawerContainer->height();
        endY = this->height();
        m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_open.png"));
    } else {
        // 打开抽屉
        startY = this->height();
        endY = this->height() - m_drawerContainer->height();
        m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_close.png"));
    }

    m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
    m_drawerAnimation->setDuration(300); // 动画时长300毫秒
    m_drawerAnimation->setStartValue(QPoint(10, startY));
    m_drawerAnimation->setEndValue(QPoint(10, endY));
    m_drawerAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 缓动曲线
    m_drawerAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    m_isDrawerOpen = !m_isDrawerOpen;
}

void MainMenuScreen::showHelp() {
    PlaceholderDialog dialog("帮助", this);
    dialog.exec();
}
void MainMenuScreen::showRecords() {
    PlaceholderDialog dialog("记录", this);
    dialog.exec();
}
void MainMenuScreen::showHighscore() {
    PlaceholderDialog dialog("最高记录", this);
    dialog.exec();
}

void MainMenuScreen::startAvalancheAnimation()
{
    qDebug() << "开始按钮被点击! 雪崩动画/视频将在这里播放。";
    // 隐藏所有UI元素
    m_startButton->hide();
    m_drawerButton->hide();
    if(m_isDrawerOpen) {
        toggleDrawer();
    }

    // 【TODO】在这里实现您的雪崩动画或者视频播放
    // 这是一个复杂的任务，现在我们只触发 startGameClicked 信号作为占位符

    // 动画/视频播放结束后，发射信号，让MainWindow切换到游戏界面
    emit startGameClicked();
}
