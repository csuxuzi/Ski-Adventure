#include "ui/MainMenuScreen.h"
#include "ui/ImageButton.h"
#include "ui/PlaceholderDialog.h"
#include "ui/HelpDialog.h" // <-- 【在这里添加新行】
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>
#include <QPainter> // 确保包含了 QPainter
#include "ui/ImageDisplayDialog.h" // <-- 【在这里添加新行】
MainMenuScreen::MainMenuScreen(QWidget *parent) : QWidget(parent)
{
    // //setupScene();//初始化游戏的背景场景（如背景图、
    // setupUI();//初始化用户界面的元素（如开始按钮、抽屉菜单等）。

    // // 将 QGraphicsView 作为此 Widget 的主要布局
    // QVBoxLayout* mainLayout = new QVBoxLayout(this);
    // mainLayout->setContentsMargins(0, 0, 0, 0);
    // mainLayout->addWidget(m_view);
    // setLayout(mainLayout);

    // // 连接开始游戏按钮的信号
    // connect(m_startButton, &QPushButton::clicked, this, &MainMenuScreen::startAvalancheAnimation);
    // 不再需要 QGraphicsScene，直接设置UI
    setupUI();

    // 连接信号
    connect(m_startButton, &QPushButton::clicked, this, &MainMenuScreen::startAvalancheAnimation);

}

// // 【新增】实现 resetUI 函数
// void MainMenuScreen::resetUI()
// {
//     m_startButton->show();
//     m_toggleButton->show();
//     // 如果抽屉是打开状态，也把它关上
//     if (m_isDrawerOpen) {
//         toggleDrawer();
//     }
// }

// void MainMenuScreen::setupScene()
// {
//     m_scene = new QGraphicsScene(0, 0, 800, 600);//创建一个800x600像素大小的图形场景-背景图片
//     m_view = new QGraphicsView(m_scene, this);//创建一个视图来观察 m_scene。
//     m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//     m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//     m_view->setRenderHint(QPainter::Antialiasing);

//     // 1. 添加背景
//     QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(QPixmap(":/assets/images/scene_background.png"));
//     m_scene->addItem(bgItem);

//     // --- 【核心修正】在这里创建带门的房子 ---

//     // 2. 加载房子和门的图片，以便获取它们的尺寸
//     QPixmap housePixmap(":/assets/images/house.png");
//     QPixmap doorPixmap(":/assets/images/house_door.png");

//     // 3. 创建房子的图形项 (QGraphicsPixmapItem)
//     QGraphicsPixmapItem* houseItem = new QGraphicsPixmapItem(housePixmap);
//     houseItem->setPos(100, 350); // 设置房子的位置
//     m_scene->addItem(houseItem);

//     // 4. 创建门的图形项
//     QGraphicsPixmapItem* doorItem = new QGraphicsPixmapItem(doorPixmap);

//     // --- 【核心修正】在这里定义您可以随时修改的门的位置偏移量 ---
//     const qreal doorOffsetX = -162; // X方向偏移量：负数向左，正数向右
//     const qreal doorOffsetY = -85; // Y方向偏移量：负数向上，正数向下

//     // 5. 计算最终的门的位置
//     //    基础位置是房子的右下角，然后应用您的自定义偏移
//     qreal doorX = houseItem->x() + housePixmap.width() + doorOffsetX;
//     qreal doorY = houseItem->y() + housePixmap.height() - doorPixmap.height() + doorOffsetY;
//     doorItem->setPos(doorX, doorY);
//     m_scene->addItem(doorItem);

//     // --- 【在这里添加以下代码块】 ---

//     // 1. 加载您想添加的静态图片
//     QPixmap titlePixmap(":/assets/images/game_title.png"); // 请确保图片路径正确

//     // 2. 创建一个用于显示该图片的图形项
//     QGraphicsPixmapItem* titleItem = new QGraphicsPixmapItem(titlePixmap);

//     // 3. 计算使其居中的位置
//     //   居中位置 = (场景宽度 - 图片宽度) / 2
//     qreal titleX = (m_scene->width() - titleItem->boundingRect().width()) / 2.0;
//     //   Y坐标可以根据您的喜好调整，这里我们让它靠近顶部
//     qreal titleY = -140; // 距离场景顶部80像素

//     // 4. 设置图片的位置并将其添加到场景中
//     titleItem->setPos(titleX, titleY);
//     m_scene->addItem(titleItem);
// }

void MainMenuScreen::setupUI()
{

    // --- 1. 加载所有需要的静态图片 ---
    m_gameTitlePixmap.load(":/assets/images/game_title.png"); // 【新增】加载标题图片
    m_housePixmap.load(":/assets/images/house.png");         // 【新增】加载房屋图片
    // --- 1. 加载所有需要的静态图片 ---
    m_characterPixmap.load(":/assets/images/sleepPlayer.png");
    m_signboardPixmap.load(":/assets/images/leftDoor.png");
    m_postPixmap.load(":/assets/images/dashBoard.png"); // 假设您的立牌图片名为 post.png

    // 对图片进行缩放 (如果需要的话)
    m_characterPixmap = m_characterPixmap.scaled(m_characterPixmap.size() * 1.0, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_signboardPixmap = m_signboardPixmap.scaled(m_signboardPixmap.size() * 0.3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_postPixmap = m_postPixmap.scaled(m_postPixmap.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_gameTitlePixmap = m_gameTitlePixmap.scaled(m_gameTitlePixmap.size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_housePixmap = m_housePixmap.scaled(m_housePixmap.size() * 0.7, Qt::KeepAspectRatio, Qt::SmoothTransformation);

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

    // --- 【新增修复】在这里只创建一次动画对象 ---
    m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
    m_drawerAnimation->setDuration(300);
    m_drawerAnimation->setEasingCurve(QEasingCurve::InOutCubic);

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

// void MainMenuScreen::toggleDrawer()
// {
//     // int startY, endY;
//     if (m_drawerAnimation && m_drawerAnimation->state() == QAbstractAnimation::Running) {
//         return;
//     }
//     m_drawerContainer->setVisible(true);
//     int startX, endX;
//     if (m_isDrawerOpen) {
//         // // 关闭抽屉
//         // startY = this->height() - m_drawerContainer->height();
//         // endY = this->height();
//         // m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_open.png"));
//         startX = m_toggleButton->pos().x();
//         endX = -m_drawerContainer->width();
//         m_toggleButton->setIcon(QIcon(":/assets/images/drawer_arrow_open.png"));
//     } else {
//         // 打开抽屉
//         // startY = this->height();
//         // endY = this->height() - m_drawerContainer->height();
//         // m_drawerButton->setIcon(QIcon(":/assets/images/drawer_arrow_close.png"));
//         startX = -m_drawerContainer->width();
//         endX = m_toggleButton->pos().x();
//         m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_close.png"));
//     }

//     m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
//     m_drawerAnimation->setDuration(300); // 动画时长300毫秒
//     // m_drawerAnimation->setStartValue(QPoint(10, startY));
//     // m_drawerAnimation->setEndValue(QPoint(10, endY));
//     // m_drawerAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 缓动曲线
//     m_drawerAnimation->setStartValue(QPoint(startX, this->height() - m_drawerContainer->height()));
//     m_drawerAnimation->setEndValue(QPoint(endX, this->height() - m_drawerContainer->height()));
//     m_drawerAnimation->setEasingCurve(QEasingCurve::InOutCubic);
//     m_drawerAnimation->start(QAbstractAnimation::DeleteWhenStopped);
//     // 【新增代码】连接 finished 信号，在动画播放完毕后将指针置空
//     // connect(m_drawerAnimation, &QPropertyAnimation::finished, this, [this]() {
//     //     m_drawerAnimation = nullptr;
//     // });
//     m_isDrawerOpen = !m_isDrawerOpen;
// }

// void MainMenuScreen::toggleDrawer()
// {
//     // 【最终修正】只在没有动画正在播放时，才创建新动画
//     if (m_drawerAnimation && m_drawerAnimation->state() == QAbstractAnimation::Running) {
//         return;
//     }

//     m_drawerContainer->setVisible(true);
//     int startX, endX;
//     if (m_isDrawerOpen) {
//         // 关闭抽屉
//         startX = m_toggleButton->pos().x();
//         endX = -m_drawerContainer->width();
//         m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_open.png"));
//     } else {
//         // 打开抽屉
//         startX = -m_drawerContainer->width();
//         endX = m_toggleButton->pos().x();
//         m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_close.png"));
//     }

//     // 创建新动画，并让Qt在播放完后自动删除它
//     m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
//     m_drawerAnimation->setDuration(300);
//     m_drawerAnimation->setStartValue(QPoint(startX, this->height() - m_drawerContainer->height()));
//     m_drawerAnimation->setEndValue(QPoint(endX, this->height() - m_drawerContainer->height()));
//     m_drawerAnimation->setEasingCurve(QEasingCurve::InOutCubic);
//     m_drawerAnimation->start(QAbstractAnimation::DeleteWhenStopped);

//     m_isDrawerOpen = !m_isDrawerOpen;
// }

void MainMenuScreen::toggleDrawer()
{
    // 1. 如果动画正在播放，则不做任何事
    if (m_drawerAnimation->state() == QAbstractAnimation::Running) {
        return;
    }

    m_drawerContainer->setVisible(true);

    // 2. 根据当前状态，设置动画的起始点和终点
    int startX, endX;
    if (m_isDrawerOpen) {
        // 配置 "关闭" 动画
        startX = m_toggleButton->pos().x();
        endX = -m_drawerContainer->width();
        m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_open.png"));
    } else {
        // 配置 "打开" 动画
        startX = -m_drawerContainer->width();
        endX = m_toggleButton->pos().x();
        m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_close.png"));
    }

    m_drawerAnimation->setStartValue(QPoint(startX, this->height() - m_drawerContainer->height()));
    m_drawerAnimation->setEndValue(QPoint(endX, this->height() - m_drawerContainer->height()));

    // 3. 启动动画 (不再使用 DeleteWhenStopped，也不再有 connect)
    m_drawerAnimation->start();

    // 4. 切换状态
    m_isDrawerOpen = !m_isDrawerOpen;
}

// void MainMenuScreen::resizeEvent(QResizeEvent *event)
// {
//     QWidget::resizeEvent(event);
//     m_startButton->move(this->width() - m_startButton->width() - 20,
//                         this->height() - m_startButton->height() - 20);
//     m_toggleButton->move(10, this->height() - m_toggleButton->height() - 10);
//     m_drawerContainer->move(-m_drawerContainer->width(), this->height() - m_drawerContainer->height());
// }

// 【修改】resizeEvent 现在只负责调整按钮的位置
void MainMenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_startButton->move(this->width() - m_startButton->width() - 20,
                        this->height() - m_startButton->height() - 20);
    m_toggleButton->move(10, this->height() - m_toggleButton->height() - 10);

    // 抽屉的位置会在 toggleDrawer 中被设置，这里无需处理
}

void MainMenuScreen::showHelp() {
    // --- 【用下面这几行替换】 ---
    HelpDialog dialog(this);
    dialog.exec();
}
void MainMenuScreen::showRecords() {
    // PlaceholderDialog dialog("记录", this);  // <-- 删除或注释掉旧代码
    // dialog.exec();

    // --- 【用下面这几行替换】 ---
    ImageDisplayDialog dialog("游戏记录",
                              ":/assets/images/records_icon.png", // 记录窗口的图标
                              ":/assets/images/敬请期待.png",   // 记录窗口的背景图
                              this);
    dialog.exec();
}

void MainMenuScreen::showHighscore() {
    // PlaceholderDialog dialog("最高记录", this); // <-- 删除或注释掉旧代码
    // dialog.exec();

    // --- 【用下面这几行替换】 ---
    ImageDisplayDialog dialog("最高得分",
                              ":/assets/images/highscore_icon.png", // 最高分窗口的图标
                              ":/assets/images/敬请期待.png",   // 最高分窗口的背景图
                              this);
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


// 【新增】重写 paintEvent 函数，在这里绘制所有静态元素
// MainMenuScreen.cpp

// 【用下面这个函数，完整替换你旧的 paintEvent 函数】
void MainMenuScreen::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 【新增】绘制从下到上变浅的渐变天空背景
    QLinearGradient skyGradient(rect().topLeft(), rect().bottomLeft());
    skyGradient.setColorAt(0.0, QColor("#2BA3C0")); // 上方颜色 (浅一点的天蓝)
    skyGradient.setColorAt(1.0, QColor("#3DAAC5")); // 下方颜色 (深一点的钢青色)
    painter.fillRect(rect(), skyGradient);

    // 2. 【修改】绘制渐变的白色雪地
    qreal snowHeight = 100; // 雪地的高度
    QRectF snowRect(0, this->height() - snowHeight, this->width(), snowHeight);
    // a. 创建一个从上到下的线性渐变
    QLinearGradient snowGradient(snowRect.topLeft(), snowRect.bottomLeft());
    // b. 设置渐变的颜色
    //    我们让雪地的上边缘（远处）带一点淡蓝色，下边缘（近处）是纯白色
    snowGradient.setColorAt(0.0, QColor("#E6E6FA")); // 上方颜色 (淡紫色/淡蓝色)
    snowGradient.setColorAt(1.0, QColor("#FFFFFF")); // 下方颜色 (纯白色)
    // c. 使用这个渐变来填充矩形
    painter.fillRect(snowRect, snowGradient);

    // --- 按照“从后往前”的顺序，绘制所有元素 ---

    // 5. 绘制角色 (逻辑不变)
    if (!m_characterPixmap.isNull()) {
        QPointF characterPos((this->width() - m_characterPixmap.width()) / 2.0-30, this->height() - snowHeight - m_characterPixmap.height() - 65);
        painter.drawPixmap(characterPos, m_characterPixmap);
    }
    // 3. 【新增】绘制房屋
    if (!m_housePixmap.isNull()) {
        // --- 在这里调整房屋的位置 ---
        QPointF housePos((this->width() - m_housePixmap.width()) / 2.0, this->height() - snowHeight - m_housePixmap.height());
        painter.drawPixmap(housePos, m_housePixmap);
    }

    // 4. 绘制告示牌 (逻辑不变)
    if (!m_signboardPixmap.isNull()) {
        QPointF signboardPos(300, this->height() - snowHeight - m_signboardPixmap.height() );
        painter.drawPixmap(signboardPos, m_signboardPixmap);
    }



    // 6. 绘制立牌 (逻辑不变)
    if (!m_postPixmap.isNull()) {
        QPointF postPos(800, this->height() - snowHeight - m_postPixmap.height());
        painter.drawPixmap(postPos, m_postPixmap);
    }

    // 7. 【新增】最后绘制标题，确保它在最上层
    if (!m_gameTitlePixmap.isNull()) {
        // --- 在这里调整标题的位置 ---
        qreal titleX = (this->width() - m_gameTitlePixmap.width()) / 2.0; // 水平居中
        qreal titleY = 40; // 距离顶部80像素
        painter.drawPixmap(QPointF(titleX, titleY), m_gameTitlePixmap);
    }
}


void MainMenuScreen::resetUI()
{
    m_startButton->show();
    m_toggleButton->show();
    if (m_isDrawerOpen) {
        toggleDrawer();
    }
}
