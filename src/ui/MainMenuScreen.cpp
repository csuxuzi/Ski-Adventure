#include "ui/MainMenuScreen.h"
#include "ui/ImageButton.h"
#include "ui/HelpDialog.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QDebug>
#include <QPainter>
#include "ui/ImageDisplayDialog.h"
MainMenuScreen::MainMenuScreen(QWidget *parent) : QWidget(parent)
{

    setupUI();

    // 现在所有按钮和控件都已经被创建好了，我们再来初始化动画
    m_drawerAnimation = new QPropertyAnimation(m_drawerContainer, "pos", this);
    m_drawerAnimation->setDuration(300);
    m_drawerAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    m_buttonAnimation = new QPropertyAnimation(m_toggleButton, "pos", this);
    m_buttonAnimation->setDuration(300);
    m_buttonAnimation->setEasingCurve(QEasingCurve::InOutCubic);



    // 最后再把所有信号和槽连接起来
    connect(m_startButton, &QPushButton::clicked, this, &MainMenuScreen::startAvalancheAnimation);
    connect(m_toggleButton, &QPushButton::clicked, this, &MainMenuScreen::toggleDrawer);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainMenuScreen::settingsClicked);
    connect(m_helpButton, &QPushButton::clicked, this, &MainMenuScreen::showHelp);
    connect(m_recordsButton, &QPushButton::clicked, this, &MainMenuScreen::showRecords);
    connect(m_highscoreButton, &QPushButton::clicked, this, &MainMenuScreen::showHighscore);
}



void MainMenuScreen::setupUI()
{

    // 加载所有需要的静态图片
    m_gameTitlePixmap.load(":/assets/images/game_title.png"); // 加载标题图片
    m_housePixmap.load(":/assets/images/house.png");         // 加载房屋图片
    // 加载所有需要的静态图片
    m_characterPixmap.load(":/assets/images/sleepPlayer.png");
    m_signboardPixmap.load(":/assets/images/leftDoor.png");
    m_postPixmap.load(":/assets/images/dashBoard.png");

    // 对图片进行缩放
    m_characterPixmap = m_characterPixmap.scaled(m_characterPixmap.size() * 1.0, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_signboardPixmap = m_signboardPixmap.scaled(m_signboardPixmap.size() * 0.3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_postPixmap = m_postPixmap.scaled(m_postPixmap.size() * 0.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_gameTitlePixmap = m_gameTitlePixmap.scaled(m_gameTitlePixmap.size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_housePixmap = m_housePixmap.scaled(m_housePixmap.size() * 0.7, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建开始按钮
    m_startButton = new ImageButton(":/assets/images/btn_start.png", this);
    m_startButton->setHoverEnabled(true);
    // 创建抽屉菜单
    // 抽屉的容器
    m_drawerContainer = new QWidget(this);
    ///创建用于打开/关闭抽屉的箭头按钮。
    m_toggleButton = new ImageButton(":/assets/images/drawer_arrow_open.png", this);
    m_drawerButtonOpenPos = QPoint(m_drawerContainer->width() + 10, m_toggleButton->y()); // 抽屉宽度 + 间距

    ///抽屉中的按钮初始化
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
    m_drawerContainer->setFixedSize(280, 80);
    m_drawerContainer->setVisible(false); // Initially hidden


}



void MainMenuScreen::toggleDrawer()
{
    // 如果动画正在播放，则不做任何事
    if (m_drawerAnimation->state() == QAbstractAnimation::Running) {
        return;
    }

    m_drawerContainer->setVisible(true);

    // 根据当前状态，设置动画的起始点和终点
    int startX, endX;
    QPoint buttonStartPos, buttonEndPos;
    if (m_isDrawerOpen) {
        // 配置 "关闭" 动画
        startX = m_toggleButton->pos().x();
        endX = -m_drawerContainer->width();
        buttonStartPos = m_toggleButton->pos();
        buttonEndPos = QPoint(10, m_toggleButton->y()); // 恢复到初始位置
        m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_open.png"));
    } else {
        // 配置 "打开" 动画
        startX = -m_drawerContainer->width();
        endX = m_toggleButton->pos().x();
        buttonStartPos = m_toggleButton->pos();
        buttonEndPos = m_drawerButtonOpenPos;
        m_toggleButton->updateIcon(QPixmap(":/assets/images/drawer_arrow_close.png"));
    }

    m_drawerAnimation->setStartValue(QPoint(startX, this->height() - m_drawerContainer->height()));
    m_drawerAnimation->setEndValue(QPoint(endX, this->height() - m_drawerContainer->height()));

    // 启动动画
    m_drawerAnimation->start();

    m_buttonAnimation->setStartValue(buttonStartPos);
    m_buttonAnimation->setEndValue(buttonEndPos);
    m_buttonAnimation->start();

    // 切换状态
    m_isDrawerOpen = !m_isDrawerOpen;
}


// resizeEvent负责调整按钮的位置
void MainMenuScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_startButton->move(this->width() - m_startButton->width() - 20,
                        this->height() - m_startButton->height() - 20);
    m_toggleButton->move(10, this->height() - m_toggleButton->height() - 10);
    m_drawerButtonOpenPos = QPoint(m_drawerContainer->width() + 10, m_toggleButton->y());
}

void MainMenuScreen::showHelp() {
    HelpDialog dialog(this);
    dialog.exec();
}
void MainMenuScreen::showRecords() {
    ImageDisplayDialog dialog("游戏记录",
                              ":/assets/images/records_icon.png", // 记录窗口的图标
                              ":/assets/images/敬请期待.png",   // 记录窗口的背景图
                              this);
    dialog.exec();
}

void MainMenuScreen::showHighscore() {
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
    m_toggleButton->hide();
    m_drawerContainer->hide();
    emit startGameClicked();
}



void MainMenuScreen::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制从下到上变浅的渐变天空背景
    QLinearGradient skyGradient(rect().topLeft(), rect().bottomLeft());
    skyGradient.setColorAt(0.0, QColor("#2BA3C0"));
    skyGradient.setColorAt(1.0, QColor("#3DAAC5"));
    painter.fillRect(rect(), skyGradient);

    // 绘制渐变的白色雪地
    qreal snowHeight = 100; // 雪地的高度
    QRectF snowRect(0, this->height() - snowHeight, this->width(), snowHeight);
    // 创建一个从上到下的线性渐变
    QLinearGradient snowGradient(snowRect.topLeft(), snowRect.bottomLeft());
    // 设置渐变的颜色
    snowGradient.setColorAt(0.0, QColor("#ffffff"));
    snowGradient.setColorAt(1.0, QColor("#082843"));
    // 使用这个渐变来填充矩形
    painter.fillRect(snowRect, snowGradient);


    // 绘制角色
    if (!m_characterPixmap.isNull()) {
        QPointF characterPos((this->width() - m_characterPixmap.width()) / 2.0-30, this->height() - snowHeight - m_characterPixmap.height() - 65);
        painter.drawPixmap(characterPos, m_characterPixmap);
    }
    // 绘制房屋
    if (!m_housePixmap.isNull()) {
        // 在这里调整房屋的位置
        QPointF housePos((this->width() - m_housePixmap.width()) / 2.0, this->height() - snowHeight - m_housePixmap.height());
        painter.drawPixmap(housePos, m_housePixmap);
    }

    // 绘制告示牌
    if (!m_signboardPixmap.isNull()) {
        QPointF signboardPos(300, this->height() - snowHeight - m_signboardPixmap.height() );
        painter.drawPixmap(signboardPos, m_signboardPixmap);
    }



    // 绘制立牌
    if (!m_postPixmap.isNull()) {
        QPointF postPos(800, this->height() - snowHeight - m_postPixmap.height());
        painter.drawPixmap(postPos, m_postPixmap);
    }

    // 最后绘制标题，确保它在最上层
    if (!m_gameTitlePixmap.isNull()) {
        qreal titleX = (this->width() - m_gameTitlePixmap.width()) / 2.0; // 水平居中
        qreal titleY = 40;
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
