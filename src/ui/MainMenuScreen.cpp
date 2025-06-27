#include "ui/MainMenuScreen.h"
#include "ui/ImageButton.h"
#include "ui/PlaceholderDialog.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>

MainMenuScreen::MainMenuScreen(QWidget *parent) : QWidget(parent)
{
    setupScene();//初始化游戏的背景场景（如背景图、
    setupUI();//初始化用户界面的元素（如开始按钮、抽屉菜单等）。

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
    m_scene = new QGraphicsScene(0, 0, 800, 600);//创建一个800x600像素大小的图形场景-背景图片
    m_view = new QGraphicsView(m_scene, this);//创建一个视图来观察 m_scene。
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
    // m_startButton->move(this->width() - m_startButton->width() - 20,
    //                     this->height() - m_startButton->height() - 20);
    m_startButton->setHoverEnabled(true);
    // --- 2. 创建抽屉菜单 ---
    // 抽屉的容器
    m_drawerContainer = new QWidget(this);
    m_drawerContainer->setStyleSheet("background-image: url(:/assets/images/drawer_background.png);");

    // 抽屉内的按钮
    // ImageButton* helpBtn = new ImageButton(":/assets/images/btn_help.png", m_drawerContainer);
    // ImageButton* recordsBtn = new ImageButton(":/assets/images/btn_records.png", m_drawerContainer);
    // ImageButton* settingsBtn = new ImageButton(":/assets/images/btn_settings.png", m_drawerContainer);
    // ImageButton* highscoreBtn = new ImageButton(":/assets/images/btn_highscore.png", m_drawerContainer);
    m_helpButton = new ImageButton(":/assets/images/btn_help.png", m_drawerContainer);
    m_recordsButton = new ImageButton(":/assets/images/btn_records.png", m_drawerContainer);
    m_settingsButton = new ImageButton(":/assets/images/btn_settings.png", m_drawerContainer);
    m_highscoreButton = new ImageButton(":/assets/images/btn_highscore.png", m_drawerContainer);

    // 使用布局管理抽屉内的按钮
    QHBoxLayout* drawerLayout = new QHBoxLayout(m_drawerContainer);
    drawerLayout->setContentsMargins(15, 15, 15, 15);
    drawerLayout->setSpacing(10);
    drawerLayout->addWidget(m_helpButton);
    drawerLayout->addWidget(m_recordsButton);
    drawerLayout->addWidget(m_settingsButton);
    drawerLayout->addWidget(m_highscoreButton);
    drawerLayout->addStretch();///添加一个弹性空间，这会把所有按钮推到布局的左侧。
    // m_drawerContainer->setLayout(drawerLayout);
    // m_drawerContainer->setFixedSize(80, 280); // 根据您的背景图调整

    // // 控制抽屉打开/关闭的按钮
    // m_drawerButton = new ImageButton(":/assets/images/drawer_arrow_open.png", this);
    // m_drawerButton->move(10, this->height() - m_drawerButton->height() - 10);
    // connect(m_drawerButton, &QPushButton::clicked, this, &MainMenuScreen::toggleDrawer);
    m_drawerContainer->setFixedSize(280, 80);
    m_drawerContainer->setVisible(false); // Initially hidden

    // 将抽屉的初始位置放在屏幕外
    // m_drawerContainer->move(10, this->height());
    ///创建用于打开/关闭抽屉的箭头按钮。
    m_toggleButton = new ImageButton(":/assets/images/drawer_arrow_open.png", this);

    // --- 3. 连接抽屉内按钮的信号 ---
    // connect(settingsBtn, &QPushButton::clicked, this, &MainMenuScreen::settingsClicked);
    // connect(helpBtn, &QPushButton::clicked, this, &MainMenuScreen::showHelp);
    // connect(recordsBtn, &QPushButton::clicked, this, &MainMenuScreen::showRecords);
    // connect(highscoreBtn, &QPushButton::clicked, this, &MainMenuScreen::showHighscore);
    connect(m_toggleButton, &QPushButton::clicked, this, &MainMenuScreen::toggleDrawer);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainMenuScreen::settingsClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &MainMenuScreen::showHelp);
    connect(m_recordsButton, &QPushButton::clicked, this, &MainMenuScreen::showRecords);
    connect(m_highscoreButton, &QPushButton::clicked, this, &MainMenuScreen::showHighscore);

}

void MainMenuScreen::toggleDrawer()
{
    // int startY, endY;
    if (m_drawerAnimation && m_drawerAnimation->state() == QAbstractAnimation::Running) {
        return;
    }
    m_drawerContainer->setVisible(true);
    int startX, endX;
    if (m_isDrawerOpen) {
        // // 关闭抽屉
        // startY = this->height() - m_drawerContainer->height();
        // endY = this->height();
        // m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_open.png"));
        startX = m_toggleButton->pos().x();
        endX = -m_drawerContainer->width();
        m_toggleButton->setIcon(QIcon(":/assets/images/drawer_arrow_open.png"));
    } else {
        // 打开抽屉
        // startY = this->height();
        // endY = this->height() - m_drawerContainer->height();
        // m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_close.png"));
        startX = -m_drawerContainer->width();
        endX = m_toggleButton->pos().x();
        m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_close.png"));
    }

    m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
    m_drawerAnimation->setDuration(300); // 动画时长300毫秒
    // m_drawerAnimation->setStartValue(QPoint(10, startY));
    // m_drawerAnimation->setEndValue(QPoint(10, endY));
    // m_drawerAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 缓动曲线
    m_drawerAnimation->setStartValue(QPoint(startX, this->height() - m_drawerContainer->height()));
    m_drawerAnimation->setEndValue(QPoint(endX, this->height() - m_drawerContainer->height()));
    m_drawerAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    m_drawerAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    // 【新增代码】连接 finished 信号，在动画播放完毕后将指针置空
    connect(m_drawerAnimation, &QPropertyAnimation::finished, this, [this]() {
        m_drawerAnimation = nullptr;
    });
    m_isDrawerOpen = !m_isDrawerOpen;
}
void MainMenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_startButton->move(this->width() - m_startButton->width() - 20,
                        this->height() - m_startButton->height() - 20);
    m_toggleButton->move(10, this->height() - m_toggleButton->height() - 10);
    m_drawerContainer->move(-m_drawerContainer->width(), this->height() - m_drawerContainer->height());
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
    // m_drawerButton->hide();
    // if(m_isDrawerOpen) {
    //     toggleDrawer();
    // }
    m_toggleButton->hide();
    m_drawerContainer->hide();
    // 【TODO】在这里实现您的雪崩动画或者视频播放
    // 这是一个复杂的任务，现在我们只触发 startGameClicked 信号作为占位符

    // 动画/视频播放结束后，发射信号，让MainWindow切换到游戏界面
    emit startGameClicked();
}
