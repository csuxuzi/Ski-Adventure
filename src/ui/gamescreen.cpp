#include "ui/GameScreen.h"
#include "game/Player.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <QtMath>
#include <QVector3D>
#include <QVBoxLayout>
#include<QKeyEvent>
#include "game/Stone.h"
#include "game/House.h"
#include "game/Seesaw.h"
#include "game/Signboard.h"
#include "ui/ImageButton.h"
#include "ui/PauseDialog.h"
#include "game/Avalanche.h"
#include "ui/GameOverDialog.h"
#include "game/Penguin.h"
#include "game/Yeti.h"
#include "game/Coin.h"
#include "fx/EffectManager.h"
#include "ui/CardSelectionDialog.h"
#include "fx/EffectManager.h"
#include "game/Tree.h"
#include "audio/AudioManager.h"

// 定义地形的一些常量，便于调整
const int TERRAIN_POINT_INTERVAL = 20; // 每个地形点的水平间距
const int TERRAIN_MIN_Y = 450;         // 地形的最低高度
const int TERRAIN_MAX_Y = 550;         // 地形的最高高度
const qreal SCROLL_SPEED = 2.0;        // 地面的滚动速度
const qreal PLAYER_VISIBLE_Y_MIN = 350.0;  // 玩家的固定y范围
const qreal PLAYER_VISIBLE_Y_MAX = 400.0;
// 定义玩家在屏幕上固定的X轴位置
const qreal PLAYER_FIXED_X = 200.0;


// 雪崩相关
const qreal AVALANCHE_START_OFFSET_X = -800.0; // 雪崩初始时在玩家身后的距离
const qreal AVALANCHE_SPEED_MULTIPLIER = 0.6;  // 雪崩初始速度是玩家的百分之多少

// 坐骑相关
const qreal PENGUIN_INITIAL_SPEED = 8.0; // 企鹅的初始速度
const qreal YETI_INITIAL_SPEED = 10.5;    // 雪怪的初始速度

const qreal PLAYER_SPEED_MULTIPLIER_ON_PENGUIN = 1.2; // 骑上企鹅后，速度变为基础速度的 1.2 倍
const qreal PLAYER_SPEED_MULTIPLIER_ON_YETI = 1.6;    // 骑上雪怪后，速度变为基础速度的 1.6 倍
// 定义角色骑上坐骑后的新重力加速度
const qreal PLAYER_GRAVITY_ON_PENGUIN = 0.3; // 角色骑上企鹅后的速度
const qreal PLAYER_GRAVITY_ON_YETI = 0.8;    // 角色骑上雪怪后的速度
// 视觉效果相关
const qreal BACKGROUND_SCROLL_RATIO = 0.1; //背景滚动速度是角色速度的10%// 【新增】定义企鹅和雪怪的最大数量上限，您可以根据需要调整这两个值
const int MAX_PENGUINS = 5;
const int MAX_YETIS = 2;


GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent), m_backgroundOffset(0),
    m_worldOffset(0), m_verticalOffset(0),
    ///初始化各种概率
    m_probNormal(0.70),
    m_probExciting(0.25),
    m_probSuperExciting(0.05),
    m_probBigStone(0.30),
    m_probHouse(0.20),
    m_probSleighInHouse(0.02),
    m_probSeesawOnCliff(0.50),
    m_probSeesawOnSteep(0.70),
    m_probPenguin_1(0.70),
    m_probPenguin_2(0.30),
    m_probPenguin_3(0.10),
    m_probYeti_1(0.30),
    m_probYeti_2(0.10),
    ///当前生成的告示牌索引
    m_signboardCount(0),
    m_lastMountGenX(0),
    m_nextSignboardDistance(1000),
    m_lastSignboardGenX(0),
    // 初始化得分变量
    m_score(0),
    m_distanceTraveledForScore(0.0),
    m_scoreMultiplier(1.0),// 默认倍率为1
    m_lastTreeX(0)
{

    m_scoreMultiplier = 1.0; // 默认倍率为1
    m_scoreMultiplierTimer = new QTimer(this);
    m_scoreMultiplierTimer->setSingleShot(true); // 设置为一次性触发
    // 连接计时器到 lambda 函数，用于时间结束后恢复倍率
    connect(m_scoreMultiplierTimer, &QTimer::timeout, this, [this]() {
        m_scoreMultiplier = 1.0;
        qDebug() << "得分倍率效果结束，已恢复为 1.0x";
    });

    // 初始化警告相关的变量
    m_isWarningVisible = false;
    m_warningScale = 1.0;
    m_warningRotation = 0.0;
    m_rotationDirection = 1.0; // 初始旋转方向设为1
    m_warningEnabled = false;  // 一开始不允许显示警告

    // 加载警告图片
    if (!m_warningPixmap.load(":/assets/images/warning.png")) {
        qWarning() << "没找到警告图片warning.png";
    }

    // 创建并设置定时器，但先不启动,在startGame启动
    m_warningDelayTimer = new QTimer(this);
    m_warningDelayTimer->setSingleShot(true); // 仍然只触发一次
    connect(m_warningDelayTimer, &QTimer::timeout, this, [this](){
        m_warningEnabled = true; // 时间到，打开警告开关
    });

    // 加载背景图，如果失败则创建一个蓝色背景作为占位符
    if (!m_backgroundPixmap.load(":/assets/images/game_background.png")) {
        qWarning("Could not load game background image, creating a placeholder.");
        m_backgroundPixmap = QPixmap(800, 600);
        m_backgroundPixmap.fill(QColor("#87CEEB"));
    }

    setupUI();

    m_cardDialog = new CardSelectionDialog(this); // 创建实例

    // 2. 创建并连接计时器，用于驱动游戏循环
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameScreen::updateGame);
    //connect(m_timer, &QTimer::timeout, this, &GameScreen::updateSnow);

    // 3. 初始化地形
    generateInitialTerrain();

    // 4. 创建并放置玩家
    resetGameState();

    setFocusPolicy(Qt::StrongFocus); // 确保 GameScreen 能接收键盘事件

    // --- 新增：在构造时调用，放置所有障碍物 ---
    //setupObstacles();
}

// 【新增】在 GameScreen.cpp 中添加 addScore 函数的完整实现
void GameScreen::addScore(int baseScore)
{
    // 1. 计算最终得分（基础分 * 倍率）
    int finalScore = static_cast<int>(baseScore * m_scoreMultiplier);

    // 2. 累加到总分
    m_score += finalScore;

    // 3. 更新主UI上的总分显示
    m_scoreLabel->setText(QString::number(m_score));

    // 4. 【关键】通知特效总管，在分数标签附近播放一个跳字动画
    QPointF scoreLabelPos = m_scoreLabel->pos();
    // 我们让跳字在分数标签的左边一点、靠下一点的位置出现
    QPointF effectPos = scoreLabelPos + QPointF(-30, m_scoreLabel->height() + 10);
    EffectManager::instance()->playFloatingScoreEffect(finalScore, effectPos);
}


// --- 【新增】UI设置函数的实现 ---
void GameScreen::setupUI()
{
    // 创建暂停按钮
    m_pauseButton = new ImageButton(":/assets/images/btn_pause.png", this);
    m_pauseButton->move(1200, 20);

    // 创建暂停对话框
    m_pauseDialog = new PauseDialog(this);
    m_pauseDialog->hide(); // 默认隐藏

    // 创建游戏结束对话框
    m_gameOverDialog = new GameOverDialog(this);
    m_gameOverDialog->hide(); // 默认隐藏

    // 创建并设置分数标签
    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setStyleSheet(
        "font-size: 28px;"
        "color: white;"
        "font-weight: bold;"
        "background-color: rgba(0, 0, 0, 100);" // 半透明黑色背景，增加可读性
        "border-radius: 8px;"
        "padding: 5px;"
        );
    m_scoreLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_scoreLabel->move(m_pauseButton->x() - 300, 20);
    m_scoreLabel->setFixedSize(200, 40);
    m_scoreLabel->setText("0"); // 初始分数

    // 创建用于显示实时信息的标签
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet(
        "font-size: 16px;"
        "color: white;"
        "background-color: rgba(0, 0, 0, 100);"
        "border-radius: 8px;"
        "padding: 5px;"
        );
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_infoLabel->setFixedSize(600, 40);
    m_infoLabel->move(m_scoreLabel->x() - 620, 20);
    m_infoLabel->setText("速度: 0.0 | 雪崩速度: 0.0 | 距离: 0.0");

    // 创建用于打开调试窗口的按钮
    m_debugButton = new ImageButton(":/assets/images/debug-icon.png", 0.23,this);
    m_debugButton->move(1120, 20); // 放置在左上角

    // 将按钮的点击信号连接到 showDebugInfo 槽
    connect(m_debugButton, &QPushButton::clicked, this, &GameScreen::showDebugInfo);


    // 3. 连接所有信号与槽
    connect(m_pauseButton, &QPushButton::clicked, this, &GameScreen::onPauseButtonClicked);
    connect(m_pauseDialog, &PauseDialog::resumeClicked, this, &GameScreen::startGame); // 继续游戏就是重启计时器
    connect(m_pauseDialog, &PauseDialog::restartClicked, this, &GameScreen::restartGame);
    // 将暂停菜单的退出按钮连接到新的 handleExit 槽
    connect(m_pauseDialog, &PauseDialog::exitClicked, this, &GameScreen::handleExit);
    connect(m_gameOverDialog, &GameOverDialog::restartClicked, this, &GameScreen::restartGame);
    // 将游戏结束菜单的退出按钮也连接到新的 handleExit 槽
    connect(m_gameOverDialog, &GameOverDialog::exitClicked, this, &GameScreen::handleExit);
}
// 暂停按钮的槽函数实现
void GameScreen::onPauseButtonClicked()
{
    stopGame(); // 停止游戏计时器
    m_pauseDialog->exec(); // 以模态方式显示暂停对话框，会阻塞游戏窗口
    // 对话框关闭后，立刻将焦点还给游戏窗口
    this->setFocus();
}
void GameScreen::handleExit()
{
    stopGame(); // 确保游戏计时器停止

    // 无论哪个对话框是可见的，都关闭它
    if (m_pauseDialog->isVisible()) {
        m_pauseDialog->accept();
    }
    if (m_gameOverDialog->isVisible()) {
        m_gameOverDialog->accept();
    }

    // 发射返回主菜单的信号
    emit backToMainMenuRequested();
}


void GameScreen::restartGame()
{
    // 确保所有计时器都已停止
    stopGame();

    // 如果有对话框是打开的，先关闭它们
    if (m_pauseDialog->isVisible()) {
        m_pauseDialog->accept();
    }
    if (m_gameOverDialog->isVisible()) {
        m_gameOverDialog->accept();
    }

    // 清理所有游戏对象
    m_snowPoints.clear();
    qDeleteAll(m_obstacles);
    m_obstacles.clear();
    qDeleteAll(m_houses);
    m_houses.clear();
    qDeleteAll(m_seesaws);
    m_seesaws.clear();
    qDeleteAll(m_signboards);
    m_signboards.clear();
    qDeleteAll(m_coins);
    m_coins.clear();
    delete m_player;
    m_player = nullptr;
    delete m_avalanche;
    m_avalanche = nullptr;
    m_terrainPatternQueue.clear();
    qDeleteAll(m_trees);
    m_trees.clear();
    m_lastTreeX = 0;

    m_signboardCount = 0;
    // 重置世界和镜头的偏移量
    m_worldOffset = 0;
    m_verticalOffset = 0;
    m_lastMountX = 0;

    // 重置分数
    m_score = 0;
    m_distanceTraveledForScore = 0.0;
    if (m_scoreLabel) {
        m_scoreLabel->setText("0");
    }

    m_scoreMultiplierTimer->stop();
    m_scoreMultiplier = 1.0;    // 将倍率重置为1


    m_nextSignboardDistance = 1000;

    // 重置警告状态
    m_warningDelayTimer->stop();
    m_warningEnabled = false;
    m_isWarningVisible = false;

    // 重新创建整个游戏世界
    generateInitialTerrain();
    resetGameState();
    //setupObstacles();

    // 重新开始游戏循环
    startGame();

    // 确保游戏窗口获得键盘焦点
    setFocus();
}

void GameScreen::startGame()
{
    // 每 16 毫秒更新一次，约等于 60 FPS
    m_timer->start(16);
    // 【新增】在这里，为新的一局游戏启动10秒的警告倒计时
    m_warningDelayTimer->start(10000);
}

void GameScreen::stopGame()
{
    m_timer->stop();
}

void GameScreen::updateGame()
{
    //背景滚动逻辑
    // 让背景滚动得比地面慢，产生视差效果
    m_backgroundOffset -= m_player->currentSpeed() * BACKGROUND_SCROLL_RATIO;

    // 玩家状态更新
    if(m_player) {
        m_player->update();
        // 更新世界偏移量
        // 世界的偏移量 = 玩家在世界中的X坐标 - 玩家在屏幕上固定的X坐标
        m_worldOffset = m_player->position().x() - PLAYER_FIXED_X;

        // 计算角色当前在屏幕上的Y坐标
        qreal playerScreenY = m_player->position().y() - m_verticalOffset;

        if (playerScreenY > PLAYER_VISIBLE_Y_MAX) {
            // 如果角色低于窗口下沿，则镜头向下移动
            m_verticalOffset = m_player->position().y() - PLAYER_VISIBLE_Y_MAX;
        } else if (playerScreenY < PLAYER_VISIBLE_Y_MIN) {
            // 如果角色高于窗口上沿，则镜头向上移动
            m_verticalOffset = m_player->position().y() - PLAYER_VISIBLE_Y_MIN;
        }


        // 分数更新逻辑
        // 累加玩家在本帧中移动的距离 (使用速度的长度来近似)
        m_distanceTraveledForScore += m_player->velocity().length();

        // 检查累计的距离是否超过了10像素的阈值
        if (m_distanceTraveledForScore >= 150.0) {
            // 计算应该加多少个10分
            int scoreIncrements = static_cast<int>(m_distanceTraveledForScore / 150.0);
            m_score += scoreIncrements * 10;

            // 从距离追踪器中减去已经计算过得分的距离
            m_distanceTraveledForScore -= scoreIncrements * 150.0;

            // 更新UI标签显示的文字
            m_scoreLabel->setText(QString::number(m_score));
        }


        checkCollisions();

        // 清理已经远在屏幕左侧的障碍物
        const qreal cleanupMargin = 200.0;


        m_trees.erase(std::remove_if(m_trees.begin(), m_trees.end(),
             [this, cleanupMargin](Tree* tree) {
             if (tree->position().x() < m_worldOffset - cleanupMargin) {
                 delete tree;
                 return true;
             }
             return false;
         }), m_trees.end());


        // 清理石头
        m_obstacles.erase(std::remove_if(m_obstacles.begin(), m_obstacles.end(),
                [this, cleanupMargin](Obstacle* obs) {
            if (obs->position().x() < m_worldOffset - cleanupMargin) {
                delete obs;
                return true;
            }
            return false;
        }), m_obstacles.end());

        // 清理房屋
        m_houses.erase(std::remove_if(m_houses.begin(), m_houses.end(),
            [this, cleanupMargin](House* house) {
            if (house->position().x() < m_worldOffset - cleanupMargin) {
                delete house;
                return true;
            }
            return false;
        }), m_houses.end());

        m_seesaws.erase(std::remove_if(m_seesaws.begin(), m_seesaws.end(),
            [this, cleanupMargin](Seesaw* seesaw) {
        if (seesaw->position().x() < m_worldOffset - cleanupMargin) {
            delete seesaw;
            return true;
        }
        return false;
        }), m_seesaws.end());
        m_yetis.erase(std::remove_if(m_yetis.begin(), m_yetis.end(),
            [this, cleanupMargin](Yeti* yeti) {
        if (yeti->position().x() < m_worldOffset - cleanupMargin) {
            delete yeti;
            return true;
        }
        return false;
        }), m_yetis.end());
        m_penguins.erase(std::remove_if(m_penguins.begin(), m_penguins.end(),
            [this, cleanupMargin](Penguin* penguin) {
        if (penguin->position().x() < m_worldOffset - cleanupMargin) {
            delete penguin;
            return true;
        }
        return false;
        }), m_penguins.end());
    }


    // 雪崩警告逻辑
    if (m_warningEnabled && m_player && m_avalanche)
    {
        // 计算玩家和雪崩的水平距离
        qreal distance = m_player->position().x() - m_avalanche->position().x();

        if (distance < 1080 * 2) {
            m_isWarningVisible = true; // 进入警告范围，显示图片

            if (distance < 540) {
                // 小于540px：放大到1.5倍并来回旋转
                m_warningScale = 1.5;
                m_warningRotation += 5.0 * m_rotationDirection; // 每次旋转5度
                // 碰到旋转边界时，改变旋转方向
                if (m_warningRotation > 15.0 || m_warningRotation < -15.0) {
                    m_rotationDirection *= -1;
                }
            } else if (distance < 1080) {
                // 540px 到 1080px 之间：平滑放大到1.2倍
                // 使用一个简单的线性插值公式，让放大过程看起来更平滑
                m_warningScale = 1.2 - (distance - 540) / (1080 - 540) * 0.2;
                m_warningRotation = 0.0; // 这个区间不旋转
            } else {
                // 1080px 到 2160px 之间：保持1.0倍大小，不旋转
                m_warningScale = 1.0;
                m_warningRotation = 0.0;
            }
        } else {
            // 超出警告范围，不显示图片
            m_isWarningVisible = false;
        }
    }


    for (Penguin* p : m_penguins) { p->update(); }
    for (Yeti* y : m_yetis) { y->update(); }

    // 更新雪崩的位置
    if(m_avalanche) {
        m_avalanche->update();
    }

    // 检查游戏是否结束
    if (m_avalanche && m_player) {
        // 计算雪崩右侧的X坐标
        qreal avalancheRightEdge = m_avalanche->position().x() + m_avalanche->width() / 2.0;
        // 判断雪崩的右侧是否触碰到玩家的X坐标（可以根据玩家的实际碰撞范围进行微调）
        if (avalancheRightEdge >= m_player->position().x()+30) {
            stopGame(); // 停止游戏
            // 在显示对话框前，设置最终分数
            m_gameOverDialog->setScore(m_score);
            m_gameOverDialog->exec(); // 显示游戏结束对话框
            return; // 立刻返回，不再执行后续更新
        }
    }


    // 每帧都检查碰撞

    checkTriggers();
    for (Obstacle* obs : m_obstacles) {
        obs->update();
    }



    // 更新雪地地形
    updateSnow();



    // 请求重新绘制界面
    EffectManager::instance()->update();

    if (m_player && m_avalanche) {
        QString playerSpeedStr = QString::number(m_player->currentSpeed(), 'f', 1);
        QString avalancheSpeedStr = QString::number(m_avalanche->velocity().length(), 'f', 1);
        qreal distance = m_player->position().x() - (m_avalanche->position().x() + m_avalanche->width() / 2.0);
        QString distanceStr = QString::number(distance, 'f', 0);

        QString mountInfo = "";
        if (m_player->isMounted()) {
            switch(m_player->currentMountType()) {
            case Player::Penguin: mountInfo = " (企鹅)"; break;
            case Player::Yeti: mountInfo = " (雪怪)"; break;
            case Player::BrokenYeti: mountInfo = " (损坏的雪怪)"; break;
            default: break;
            }
        }

        m_infoLabel->setText(QString("角色速度: %1%2 | 雪崩速度: %3 | 距离: %4")
         .arg(playerSpeedStr)
         .arg(mountInfo)
         .arg(avalancheSpeedStr)
         .arg(distanceStr));
    }

    update();

    if(m_player) {
        m_worldOffset = m_player->position().x() - PLAYER_FIXED_X;
    }

}



GameScreen::TerrainType GameScreen::getTerrainTypeAt(qreal x_pos)
{
    // 这是一个简化的实现，实际需要更复杂的逻辑来根据 m_terrainPatternQueue 判断
    auto info = getPathInfoAt(m_snowPath, x_pos);
    qreal angle = abs(info.second);
    if(angle > 40) return Cliff;
    if(angle > 25) return Steep;
    return Gentle;
}

// 判断当前窗口是否是地形模式的最后一段
bool GameScreen::isLastSegmentOfPattern(qreal x_pos)
{
    // 如果模式队列为空，我们就认为当前是最后一段
    return m_terrainPatternQueue.isEmpty();
}

// --- 新增：实现键盘事件处理函数 ---
void GameScreen::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return; // 忽略长按的重复事件
    }

    if (m_player) {
        // 如果玩家正在摔倒，任何按键都是为了让他快点起来
        if (m_player->currentState == Player::Crashing) {
            m_player->reduceCrashTime();
            return; // 不再执行后续的跳跃等操作
        }

        switch (event->key()) {
        case Qt::Key_Space:
            m_player->jump();
            break;
        default:
            QWidget::keyPressEvent(event);
        }
    }
}


void GameScreen::updateSnow()
{
    // 根据雪崩的位置来清理雪地
    if (m_avalanche) {
        // 只要雪崩存在，就确保它脚下的地形不被删除
        while (m_snowPoints.size() > 2 && m_snowPoints[1].x() < m_avalanche->position().x()-200) {
            m_snowPoints.removeFirst();
        }
    } else {
        // 如果雪崩还没被创建，则沿用旧的、基于玩家位置的清理方式
        while (m_snowPoints.size() > 2 && m_snowPoints[1].x() < m_worldOffset) {
            m_snowPoints.removeFirst();
        }
    }

    // 如果地形模式队列为空，则生成下一个模式
    if (m_terrainPatternQueue.isEmpty()) {
        generateNextPattern();
    }

    // 如果下一个地形片段未生成，则从队列中取一个类型来生成
    if (m_nextTerrainSegment.isEmpty()) {
        if (!m_terrainPatternQueue.isEmpty()) {
            TerrainType nextType = m_terrainPatternQueue.dequeue();
            qreal segmentStartX = m_snowPoints.last().x(); // 记录片段起始X
            switch (nextType) {
            case Gentle:
                generateGentleSlope(m_nextTerrainSegment, m_snowPoints.last());
                break;
            case Steep:
                generateSteepSlope(m_nextTerrainSegment, m_snowPoints.last());
                break;
            case Cliff:
                generateCliff(m_nextTerrainSegment, m_snowPoints.last());
                break;
            }

            // 生成地形后，立刻为其填充素材
            if(!m_nextTerrainSegment.isEmpty()){
                placeObjectsForSegment(nextType, m_nextTerrainSegment);
            }
        }
    }

    // 如果下一个片段已就绪，且屏幕需要，则拼接到主地形上
    if (!m_nextTerrainSegment.isEmpty() && m_snowPoints.last().x() < m_worldOffset + width() + TERRAIN_POINT_INTERVAL) {
        // 使用 move 提高效率，避免深拷贝
        m_snowPoints.append(std::move(m_nextTerrainSegment));
        m_nextTerrainSegment.clear();
    }

    // 更新最终用于绘制的路径
    updateSnowPath();

}


void GameScreen::placeObjectsForSegment(TerrainType type, const QList<QPointF>& segmentPoints)
{
    if (segmentPoints.isEmpty()) return;

    qreal segmentStartX = segmentPoints.first().x();
    qreal segmentEndX = segmentPoints.last().x();

    // 在游戏刚开始的一小段距离内，不放置任何复杂物件，给玩家缓冲空间
    if (segmentEndX < 1500) return;

    // 创建一个列表，用来记录所有已放置物件的中心X坐标
    QList<qreal> occupiedXPositions;
    // 2. 定义所有物件之间必须保持的最小安全距离
    const qreal MIN_SPACING = 250.0;

    // 3. 创建一个查询函数，任何物件在放置前都必须调用它
    auto isAreaClear = [&](qreal candidateX) {
        for (qreal occupiedX : occupiedXPositions) {
            if (qAbs(candidateX - occupiedX) < MIN_SPACING) {
                return false; // 如果距离太近，则区域不干净
            }
        }
        return true; // 区域干净
    };

    // --- 统一生成逻辑 ---
    // 以窗口为单位，在地形片段中循环生成素材
    qreal windowStartX = floor(segmentStartX / 1080.0) * 1080.0;
    if (windowStartX < segmentStartX) {
        windowStartX += 1080.0;
    }

    while (windowStartX < segmentEndX) {

        // 1. 告示牌
        // 每10800个单位（大约10个屏幕宽度）尝试生成一个
        if (windowStartX >= m_lastSignboardGenX + 10800.0) {
            if (type == Steep && isLastSegmentOfPattern(windowStartX)) {
                qreal signboardX = windowStartX + 980.0;
                if (isAreaClear(signboardX)) {
                    Signboard* sign = new Signboard(this);
                    sign->setPosition(getTerrainInfoAt(signboardX).first);
                    sign->setDistance(m_nextSignboardDistance);
                    m_nextSignboardDistance += 1000;
                    m_signboards.append(sign);
                    occupiedXPositions.append(signboardX); // 登记位置
                    m_lastSignboardGenX = windowStartX;

                    m_signboardCount++;

                    // 在告示牌周围生成关联的金币
                    if (m_signboardCount % 5 == 1) {
                        Coin* leftCoin = new Coin(this);
                        leftCoin->setPosition(sign->position() - QPointF(200, 20));
                        m_coins.append(leftCoin);
                        Coin* topCoin = new Coin(this);
                        topCoin->setPosition(sign->position() - QPointF(0, 190));
                        m_coins.append(topCoin);
                    }
                }
            }
        }

        // 2. 房屋 (只在平缓地形)
        if (type == Gentle && QRandomGenerator::global()->generateDouble() < m_probHouse) {
            qreal houseCandidateX = windowStartX + QRandomGenerator::global()->bounded(200, 880);
            if (isAreaClear(houseCandidateX)) {
                House* house = new House(this);
                qDebug()<<"房屋位置"<<houseCandidateX;
                house->setPosition(getTerrainInfoAt(houseCandidateX).first);
                m_houses.append(house);
                occupiedXPositions.append(houseCandidateX); // 登记位置
            }
        }

        // 3. 翘板 (只在陡峭或悬崖地形)
        if ((type == Steep || type == Cliff) && QRandomGenerator::global()->generateDouble() < m_probSeesawOnSteep) {
            qreal seesawCandidateX = windowStartX + 100.0 + QRandomGenerator::global()->bounded(-50, 50);
            if (isAreaClear(seesawCandidateX)) {
                Seesaw* seesaw = new Seesaw(this);
                qDebug()<<"翘板位置"<<seesawCandidateX;
                seesaw->setPosition(getTerrainInfoAt(seesawCandidateX).first);
                m_seesaws.append(seesaw);
                occupiedXPositions.append(seesawCandidateX); // 登记位置
            }
        }

        // 4. 坐骑 (企鹅和雪怪)
        generateMountsInWindow(windowStartX);


        // 5. 石头 (用于填充剩余空间)
        int stoneCount = QRandomGenerator::global()->bounded(0, 3); // 每屏最多生成0-2个石头
        for (int i = 0; i < stoneCount; ++i) {
            qreal stoneCandidateX = windowStartX + QRandomGenerator::global()->bounded(100, 980);
            if (isAreaClear(stoneCandidateX)) {
                Stone* newStone = new Stone(Stone::Small, this);
                qDebug()<<"第"<<i<<"个石头："<<stoneCandidateX;
                newStone->setPosition(getTerrainInfoAt(stoneCandidateX).first);
                m_obstacles.append(newStone);
                occupiedXPositions.append(stoneCandidateX); // 登记位置
            }
        }

        windowStartX += 1080.0;
    }


    // 6. 树木 (纯背景装饰，允许重叠)
    qreal treeX = QRandomGenerator::global()->bounded(static_cast<int>(segmentStartX), static_cast<int>(segmentEndX));
    qDebug()<<"树木位置"<<treeX;
    Tree* tree = new Tree(this);
    tree->setPosition(getTerrainInfoAt(treeX).first);
    m_trees.append(tree);
    qDebug()<<"一次调用结束；";
}



void GameScreen::generateMountsInWindow(qreal windowStartX)
{
    // 坐骑在游戏开始一段距离后才出现
    if (windowStartX < 2000) return;
    const int MAX_PENGUINS = 5;
    const int MAX_YETIS = 2;

    // 企鹅生成
    if (m_penguins.size() < MAX_PENGUINS) {
        double pChoice = QRandomGenerator::global()->generateDouble();
        if (pChoice < m_probPenguin_3) { // 10% 概率生成3个
            for(int i = 0; i < 3; ++i) {
                if (m_penguins.size() >= MAX_PENGUINS) break;
                Penguin* p = new Penguin(this);
                p->setPosition(getTerrainInfoAt(windowStartX + 200 + i * 200).first - QPointF(0, 20));
                m_penguins.append(p);
            }
        } else if (pChoice < m_probPenguin_2) { // 30% 概率生成2个
            for(int i = 0; i < 2; ++i) {
                if (m_penguins.size() >= MAX_PENGUINS) break;
                Penguin* p = new Penguin(this);
                p->setPosition(getTerrainInfoAt(windowStartX + 300 + i * 200).first - QPointF(0, 20));
                m_penguins.append(p);
            }
        } else if (pChoice < m_probPenguin_1) { // 70% 概率生成1个
            if (m_penguins.size() < MAX_PENGUINS) {
                Penguin* p = new Penguin(this);
                p->setPosition(getTerrainInfoAt(windowStartX + 400).first - QPointF(0, 20));
                m_penguins.append(p);
            }
        }
    }


    // 雪怪生成
    if (m_yetis.size() < MAX_YETIS) {
        double yChoice = QRandomGenerator::global()->generateDouble();
        if (yChoice < m_probYeti_2) { // 10% 概率生成2个
            for(int i = 0; i < 2; ++i) {
                if (m_yetis.size() >= MAX_YETIS) break;
                Yeti* y = new Yeti(this);
                y->setPosition(getTerrainInfoAt(windowStartX + 150 + i * 500).first - QPointF(0, 40));
                m_yetis.append(y);
            }
        } else if (yChoice < m_probYeti_1) { // 30% 概率生成1个
            if (m_yetis.size() < MAX_YETIS) {
                Yeti* y = new Yeti(this);
                y->setPosition(getTerrainInfoAt(windowStartX + 600).first - QPointF(0, 40));
                m_yetis.append(y);
            }
        }
    }
}


void GameScreen::generateNextPattern()
{
    m_terrainPatternQueue.clear();
    double choice = QRandomGenerator::global()->generateDouble();

    if (choice < m_probSuperExciting) { // “超爽”模式 - 平-悬-悬-陡-陡-陡-陡-陡-陡
        qDebug()<<"抽中超爽";
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Cliff);
        m_terrainPatternQueue.enqueue(Cliff);
        for(int i = 0; i < 6; ++i) m_terrainPatternQueue.enqueue(Steep);
    } else if (choice < m_probSuperExciting + m_probExciting) { // “爽”模式 - 平-陡-陡-陡-平-平-陡
        qDebug()<<"抽中爽";
        m_terrainPatternQueue.enqueue(Gentle);
        for(int i = 0; i < 3; ++i) m_terrainPatternQueue.enqueue(Steep);
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Steep);
    } else { // “一般”模式 - 平-平-陡-平-平-陡
        qDebug()<<"抽中一般";
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Steep);
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Gentle);
        m_terrainPatternQueue.enqueue(Steep);
    }
}

void GameScreen::generateInitialTerrain()
{
    m_snowPoints.clear();
    m_nextTerrainSegment.clear();
    m_terrainPatternQueue.clear();

    // 游戏开始时，总是先生成一段平缓的雪地
    // 从雪地偏移量开始生成，可以避免游戏开始时镜头突变或穿模的问题
    generateGentleSlope(m_snowPoints, QPointF(AVALANCHE_START_OFFSET_X, 500));
    updateSnowPath();
}

void GameScreen::updateSnowPath()
{
    if (m_snowPoints.size() < 2) return;

    // 开始构建路径
    m_snowPath = QPainterPath();
    // 计算当前镜头的实际底部Y坐标
    qreal bottomY = m_verticalOffset + height();
    // 将起点移动到屏幕左下角 (使用新的底部坐标)
    m_snowPath.moveTo(m_snowPoints.first().x(), bottomY);
    // 画一条线到第一个地形点
    m_snowPath.lineTo(m_snowPoints.first());
    // 将所有地形点连接成一条曲线
    for (int i = 1; i < m_snowPoints.size(); ++i) {
        m_snowPath.lineTo(m_snowPoints[i]);
    }
    // 从最后一个地形点画一条线到屏幕右下角 (使用新的底部坐标)
    m_snowPath.lineTo(m_snowPoints.last().x(), bottomY);
    // 闭合路径，形成一个封闭的多边形
    m_snowPath.closeSubpath();
}

void GameScreen::resetGameState()
{
    m_player = new Player(this);

    // 将玩家放置在屏幕大约 1/4 的位置
    qreal player_x = width() / 4.0;

    // 获取该位置的地形信息（Y坐标和坡度）
    auto terrain_info = getTerrainInfoAt(player_x);

    // 设置玩家的初始位置和旋转checkCollisions
    m_player->setPosition(terrain_info.first);
    m_player->setRotation(terrain_info.second);


    // --- 【核心修改】根据坡度设置初始速度方向，大小由玩家自己决定 ---
    qreal initialAngleRad = qDegreesToRadians(m_player->rotation());
    QVector2D initial_direction(qCos(initialAngleRad), qSin(initialAngleRad));

    // 用方向乘以玩家自己的当前速度来设置速度矢量
    m_player->setVelocity(initial_direction * m_player->currentSpeed());

    m_avalanche = new Avalanche(this, this);
    // a. 计算雪崩的初始X坐标
    qreal avalancheStartX = m_player->position().x() + AVALANCHE_START_OFFSET_X;
    // b. 获取该位置的地形信息
    auto avalancheTerrainInfo = getTerrainInfoAt(avalancheStartX);
    // c. 设置雪崩的初始位置
    m_avalanche->setPosition(avalancheTerrainInfo.first);

    // d. 【核心】设置雪崩的初始速度，使其方向与初始地形平行
    qreal initialPlayerSpeed = m_player->velocity().length();
    qreal initialAvalancheSpeed = initialPlayerSpeed * AVALANCHE_SPEED_MULTIPLIER;
    qreal initialAvalancheAngleRad = qDegreesToRadians(avalancheTerrainInfo.second);
    QVector2D initialAvalancheDirection(qCos(initialAvalancheAngleRad), qSin(initialAvalancheAngleRad));

    m_avalanche->setVelocity(initialAvalancheDirection * initialAvalancheSpeed);
}

QPair<QPointF, qreal> GameScreen::getTerrainInfoAt(qreal x_pos)
{
    if (m_snowPoints.size() < 2) {
        return qMakePair(QPointF(x_pos, height() / 2.0), 0.0);
    }

    // 找到包围 x_pos 的两个地形点
    for (int i = 0; i < m_snowPoints.size() - 1; ++i) {
        const QPointF& p1 = m_snowPoints[i];
        const QPointF& p2 = m_snowPoints[i+1];

        if (p1.x() <= x_pos && p2.x() >= x_pos) {
            // 使用线性插值计算精确的Y坐标
            qreal t = (x_pos - p1.x()) / (p2.x() - p1.x());
            qreal y_pos = p1.y() + t * (p2.y() - p1.y());

            // 使用 atan2 计算坡度的角度（弧度转角度）
            qreal angle = qAtan2(p2.y() - p1.y(), p2.x() - p1.x()) * 180 / M_PI;

            return qMakePair(QPointF(x_pos, y_pos), angle);
        }
    }

    // 如果没找到（例如 x_pos 在所有点之外），返回最后一个点的信息
    return qMakePair(m_snowPoints.last(), 0.0);
}

void GameScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // --- 绘制背景 (有视差滚动) ---
    painter.save();
    qreal bg_x = fmod(m_backgroundOffset, width());

    int x1_base = static_cast<int>(bg_x);
    int w = width();

    // 3. 【最关键】绘制三张图，每一张都比前一张“提前”2个像素，形成覆盖
    //    绘制顺序：左 -> 中 -> 右，后画的会盖住先画的
    painter.drawPixmap(x1_base - w, 0, w, height(), m_backgroundPixmap);       // 左边的图，位置不变
    painter.drawPixmap(x1_base - 2, 0, w, height(), m_backgroundPixmap);      // 中间的图，提前2像素，覆盖左图
    painter.drawPixmap(x1_base + w - 4, 0, w, height(), m_backgroundPixmap); // 右边的图，再提前2像素，覆盖中图

    painter.restore();


    // --- 【核心修改】绘制游戏世界 ---
    painter.save();
    // 将整个坐标系向左移动 worldOffset 的距离
    painter.translate(-m_worldOffset, -m_verticalOffset);

    // 在这个被移动过的坐标系里，正常绘制所有游戏对象即可
    // 1. 绘制雪地
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawPath(m_snowPath);

    // 2. 绘制玩家
    if(m_player) {
        m_player->draw(&painter);
    }

    // --- 新增：绘制所有障碍物 ---
    for (Obstacle* obs : m_obstacles) {
        obs->draw(&painter);
    }
    for (House* house : m_houses) {
        house->draw(&painter);
    }

    for (Tree* tree : m_trees) {
        tree->draw(&painter);
    }

    for (Seesaw* seesaw : m_seesaws) {
        seesaw->draw(&painter);
    }

    for (Signboard* signboard : m_signboards) {
        signboard->draw(&painter);
    }
    // 3. 在这里绘制未来的企鹅、雪怪等...
    //   (它们也会被自动正确地放置在滚动的世界中)

    // --- 【新增】绘制所有坐骑 ---
    for (Penguin* p : m_penguins) { p->draw(&painter); }
    for (Yeti* y : m_yetis) { y->draw(&painter); }

    for (Coin* coin : m_coins) {
        coin->draw(&painter);
    }

    // --- 【新增】绘制雪崩 ---
    if(m_avalanche) {
        m_avalanche->draw(&painter);
    }

    // 因为 EffectManager 存储的是世界坐标，所以它不需要在滚动的坐标系里绘制
    EffectManager::instance()->draw(&painter);

    painter.restore(); // 恢复坐标系，不影响后续的UI绘制


    // 【核心修正2】在UI层(不受滚动影响)绘制文字特效
    EffectManager::instance()->drawTextEffects(&painter);


    // 【新增】在所有东西的最上层，绘制UI元素，比如这个警告图片
    if (m_isWarningVisible && !m_warningPixmap.isNull())
    {
        painter.save(); // 保存当前画笔状态，以免影响其他UI

        QPointF drawPos(50, 50); // 我们把图片画在左上角 (50, 50) 的位置
        QPointF center = drawPos + QPointF(m_warningPixmap.width() / 2.0, m_warningPixmap.height() / 2.0);

        // 关键的绘图三连：位移 -> 旋转 -> 缩放
        painter.translate(center); // 1. 将坐标系原点移动到图片中心
        painter.rotate(m_warningRotation);  // 2. 绕着新的原点（图片中心）旋转
        painter.scale(m_warningScale, m_warningScale); // 3. 以图片中心为基准进行缩放

        // 将坐标系移回去，然后绘制图片
        painter.translate(-center + drawPos);
        painter.drawPixmap(QPointF(0,0), m_warningPixmap);

        painter.restore(); // 恢复画笔状态
    }
    // --- 在这里可以绘制固定的UI元素，如分数、生命值等 ---
    // painter.drawText(20, 40, "Score: ...");

    // Q_UNUSED(event);
    // QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，让曲线更平滑


}

// --- 新增：放置障碍物的实现 ---
void GameScreen::setupObstacles()
{




    // 定义要生成的区域范围
    const qreal placementStart = m_lastObstacleX;
    const qreal placementEnd = m_worldOffset + width() * 2; // 生成到屏幕外一个屏幕的距离
    const int placementInterval = 350; // 调整生成密度

    for (qreal currentX = placementStart; currentX < placementEnd; currentX += placementInterval) {
        qreal randomX = currentX + QRandomGenerator::global()->bounded(-150, 150);
        if (randomX < m_lastObstacleX) continue; // 避免在已经生成过的地方重复生成

        QPointF terrainPos = getTerrainInfoAt(randomX).first;
        double choice = QRandomGenerator::global()->generateDouble();

        // 【已修复】使用正确的概率进行判断
        if (choice < 0.10) { // 10% 概率放置房屋
            House* house = new House(this);
            house->setPosition(terrainPos);
            m_houses.append(house);
        } else if (choice < 0.25) { // 15% 概率
            Seesaw* seesaw = new Seesaw(this);
            seesaw->setPosition(terrainPos);
            //seesaw->setRotation(QRandomGenerator::global()->bounded(-15, 15));
            m_seesaws.append(seesaw);
        } else if (choice < 0.50) { // 25% 概率
            Stone* stone = new Stone(Stone::Small, this);
            stone->setPosition(terrainPos);
            m_obstacles.append(stone);
        } else if (choice < 0.60) { // 10% 概率
            Penguin* penguin = new Penguin(this);
            penguin->setPosition(QPointF(randomX, terrainPos.y() - 50));
            penguin->setVelocity(QVector2D(PENGUIN_INITIAL_SPEED, 0));
            m_penguins.append(penguin);
        } else if (choice < 0.65) { // 5% 概率
            Yeti* yeti = new Yeti(this);
            yeti->setPosition(QPointF(randomX, terrainPos.y() - 80));
            yeti->setVelocity(QVector2D(YETI_INITIAL_SPEED, 0));
            m_yetis.append(yeti);
        }
    }
    // 更新已生成到的最远距离
    m_lastObstacleX = placementEnd;

    qDebug() << "地图素材已重新生成！";
}
void GameScreen::setupMounts()
{
    const int MOUNT_INTERVAL = 1080; // 每隔1080px尝试生成一次
    while (m_lastMountX < m_worldOffset + width() * 1.5) {
        qreal candidateX = m_lastMountX + MOUNT_INTERVAL;
        m_lastMountX = candidateX; // 更新循环变量

        if (candidateX < 2000) continue; // 坐骑稍晚出现

        qDebug()<<"企鹅个数："<<m_penguins.size();
        qDebug()<<"雪怪个数："<<m_yetis.size();


        // 企鹅生成
        if (m_penguins.size() < MAX_PENGUINS) {
            double pChoice = QRandomGenerator::global()->generateDouble();
            if (pChoice < m_probPenguin_3) { // 10% 3个
                for(int i = 0; i < 3; ++i) { Penguin* p = new Penguin(this); p->setPosition(getTerrainInfoAt(candidateX + i * 200).first - QPointF(0, 20)); m_penguins.append(p); }
            } else if (pChoice < m_probPenguin_2) { // 30% 2个
                for(int i = 0; i < 2; ++i) { Penguin* p = new Penguin(this); p->setPosition(getTerrainInfoAt(candidateX + i * 200).first - QPointF(0, 20)); m_penguins.append(p); }
            } else if (pChoice < m_probPenguin_1) { // 70% 1个
                Penguin* p = new Penguin(this); p->setPosition(getTerrainInfoAt(candidateX).first - QPointF(0, 20)); m_penguins.append(p);
            }
        }

        // 雪怪生成
        if (m_yetis.size() < MAX_YETIS) {
            double yChoice = QRandomGenerator::global()->generateDouble();
            if (yChoice < m_probYeti_2) { // 10% 2个
                for(int i = 0; i < 2; ++i) { Yeti* y = new Yeti(this); y->setPosition(getTerrainInfoAt(candidateX + 150 + i * 500).first - QPointF(0, 40)); m_yetis.append(y); }
            } else if (yChoice < m_probYeti_1) { // 30% 1个
                Yeti* y = new Yeti(this); y->setPosition(getTerrainInfoAt(candidateX + 300).first - QPointF(0, 40)); m_yetis.append(y);
            }
        }
    }
}

// --- 【最终修正版】碰撞检测实现 (无 GOTO) ---
void GameScreen::checkCollisions()
{
    if (!m_player) return;

    bool isPlayerOnASurface = false;
    bool playerCollisionHandled = false; // 新的布尔标志
    bool playerJustMounted = false;
    // 【核心修复】只有在玩家不处于摔倒或站立状态时，才进行坐骑碰撞检测
    if (m_player->currentState != Player::Crashing && m_player->currentState != Player::StandingUp)
    {
        // --- 阶段零：玩家与坐骑的碰撞（安全删除模式 + 优先级判断）---

        // 1. 优先检测与雪怪的碰撞 (最高优先级)
        if (m_player->currentMountType() <= Player::Yeti)
        {
            for (auto it = m_yetis.begin(); it != m_yetis.end(); ++it)
            {
                if (m_player->collisionRect().intersects((*it)->collisionRect()))
                {
                    addScore(100);
                    AudioManager::instance()->playSoundEffect(SfxType::YetiRoar);
                    m_player->rideMount(Player::Yeti, (*it)->getAnimationFrames(), m_player->baseSpeed() * PLAYER_SPEED_MULTIPLIER_ON_YETI, PLAYER_GRAVITY_ON_YETI);
                    (*it)->deleteLater();
                    it = m_yetis.erase(it);
                    playerJustMounted = true;
                    m_player->startInvincibility();
                    break;
                }
            }
        }

        // 2. 如果没有骑上雪怪，再检测与企鹅的碰撞
        if (!playerJustMounted && m_player->currentMountType() <= Player::Penguin)
        {
            for (auto it = m_penguins.begin(); it != m_penguins.end(); ++it)
            {
                if (m_player->collisionRect().intersects((*it)->collisionRect()))
                {
                    addScore(50);
                    AudioManager::instance()->playSoundEffect(SfxType::PenguinChirp);
                    m_player->rideMount(Player::Penguin, (*it)->getAnimationFrames(), m_player->baseSpeed() * PLAYER_SPEED_MULTIPLIER_ON_PENGUIN, PLAYER_GRAVITY_ON_PENGUIN);
                    (*it)->deleteLater();
                    it = m_penguins.erase(it);
                    playerJustMounted = true;
                    m_player->startInvincibility();
                    break;
                }
            }
        }
    }


    // --- 【重要】如果刚刚骑上坐骑，更新玩家状态并跳过环境碰撞 ---
    if (playerCollisionHandled) {
        isPlayerOnASurface = true;
        //goto end_all_surface_checks;
    }

    for (House* house : m_houses) {
        // // A. 检测天花板碰撞 (从下往上)
        // QPointF leftCorner = house->getRoofLeftCornerWorldPosition();
        // QPointF rightCorner = house->getRoofRightCornerWorldPosition();
        // if (m_player->velocity().y() < 0 &&
        //     m_player->position().x() > leftCorner.x() &&
        //     m_player->position().x() < rightCorner.x() &&
        //     m_player->collisionRect().top() < leftCorner.y()) {

        //     // 撞到天花板，硬碰撞
        //     QPointF playerPos = m_player->position();
        //     playerPos.setY(leftCorner.y() + m_player->collisionRect().height());
        //     m_player->setPosition(playerPos);
        //     m_player->setVelocity(QVector2D(m_player->velocity().x(), 0));
        //     house->shatter(QPointF(m_player->position().x(), leftCorner.y()));
        //     playerCollisionHandled = true; // 标记已处理
        //     break; // 跳出房屋循环
        // }

        // B. 检测屋顶表面碰撞 (从上往下)
        QPainterPath roofPath = house->getRoofPath();
        auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
        if (roofInfo.first.y() != 0 && fabs(m_player->position().y() - roofInfo.first.y()) < 5.0) {



            QPointF peakWorldPos = house->getRoofPeakWorldPosition();
            if (m_player->position().x() < peakWorldPos.x()) {
                // 左侧滑行
                isPlayerOnASurface = true;
                QPointF playerPos = m_player->position();
                playerPos.setY(roofInfo.first.y());
                m_player->setPosition(playerPos);
                m_player->setRotation(roofInfo.second);
                QVector2D airVelocity = m_player->velocity();
                QVector2D groundDirection(qCos(qDegreesToRadians(roofInfo.second)), qSin(qDegreesToRadians(roofInfo.second)));
                m_player->setVelocity(groundDirection * m_player->currentSpeed());
            } else {
                house->shatter(m_player->position());
                // 右侧挤压
                QPointF playerPos = m_player->position();
                playerPos.setY(roofInfo.first.y());
                m_player->setPosition(playerPos);
            }
            playerCollisionHandled = true; // 标记已处理
            break; // 跳出房屋循环
        }
    }

    // --- 阶段二：如果不在房屋上，再检测与翘板的碰撞 ---
    if (!playerCollisionHandled) {
        for (Seesaw* seesaw : m_seesaws) {
            if (seesaw->currentState() == Seesaw::Shattered) continue;

            QPainterPath plankPath = seesaw->getPlankPath();
            auto plankInfo = getPathInfoAt(plankPath, m_player->position().x());
            qreal plankY = plankInfo.first.y(); // 先获取翘板表面的Y坐标

            // 【核心修正】使用更稳定的碰撞判断逻辑
            // 条件1: 玩家在翘板的水平范围内 (plankY != 0)
            // 条件2: 玩家的脚底在翘板表面或已穿过其表面
            // 条件3: 玩家正在向下落或水平移动
            if (plankY != 0 && m_player->position().y() >= plankY - 1.0 && m_player->velocity().y() >= 0) {

                // 既然接触了，就触发破碎效果


                qreal plankAngle = plankInfo.second;
                // --- 【核心修改】调用新函数来计算撞击角度 ---
                // qDebug()<<"速度的矢量为:"<<m_player->velocity();
                // qDebug()<<"速度的夹角为:"<<m_player->rotation();
                qreal impactAngle = calculateImpactAngle(m_player->velocity(), plankAngle);
                //qDebug()<<"两者的夹角为:"<<impactAngle;
                // 角度判断逻辑保持不变
                if (fabs(impactAngle) > 60.0) { // 这里使用之前确定的80度
                    // 角度过陡，执行摔倒逻辑
                    seesaw->shatter(m_player->position());
                    QPointF playerPos = m_player->position();
                    playerPos.setY(plankY);
                    m_player->setPosition(playerPos);
                    m_player->crash(); // 调用摔倒函数
                } else {
                    // 正常滑行
                    isPlayerOnASurface = true;
                    QPointF playerPos = m_player->position();
                    playerPos.setY(plankY);
                    m_player->setPosition(playerPos);
                    m_player->setRotation(plankAngle);

                    qreal currentSpeed = m_player->velocity().length();
                    QVector2D groundDirection(qCos(qDegreesToRadians(plankAngle)), qSin(qDegreesToRadians(plankAngle)));
                    m_player->setVelocity(groundDirection * currentSpeed);
                }

                playerCollisionHandled = true;
                break;
            }
        }
    }
    if (!playerCollisionHandled) {
        for (Signboard* sign : m_signboards) {
            if (sign->currentState() == Signboard::Shattered) continue;

            QPainterPath boardPath = sign->getBoardPath();
            auto boardInfo = getPathInfoAt(boardPath, m_player->position().x());
            if (boardInfo.first.y() != 0 && fabs(m_player->position().y() - boardInfo.first.y()) < 5.0) {
                // 【完全复刻的逻辑】根据角度判断滑行或摔倒
                qreal boardAngle = boardInfo.second;
                qreal impactAngle = calculateImpactAngle(m_player->velocity(), boardAngle);
                if (fabs(impactAngle) > 60.0) {
                    sign->shatter(m_player->position());
                    QPointF playerPos = m_player->position();
                    playerPos.setY(boardInfo.first.y());
                    m_player->setPosition(playerPos);
                    m_player->crash(); // 调用摔倒函数
                } else {
                    isPlayerOnASurface = true;
                    QPointF playerPos = m_player->position();
                    playerPos.setY(boardInfo.first.y());
                    m_player->setPosition(playerPos);
                    m_player->setRotation(boardAngle);
                    qreal currentSpeed = m_player->velocity().length();
                    QVector2D groundDirection(qCos(qDegreesToRadians(boardAngle)), qSin(qDegreesToRadians(boardAngle)));
                    m_player->setVelocity(groundDirection * currentSpeed);
                }

                playerCollisionHandled = true;
                break;
            }
        }
    }
    // --- 【新增】检测所有企鹅的碰撞 ---
    for (Penguin* p : m_penguins) {
        // A. 检测与地形的碰撞 (与玩家逻辑类似)
        auto terrainInfo = getTerrainInfoAt(p->position().x());
        if (p->position().y() >= terrainInfo.first.y()) {
            p->onGround = true;
            QPointF pos = p->position();
            pos.setY(terrainInfo.first.y());
            p->setPosition(pos);
            // 恢复水平速度，垂直速度清零
            p->setVelocity(QVector2D(p->velocity().x(), 0));
        } else {
            p->onGround = false;
        }

        // B. 检测与房屋、翘板、告示牌的碰撞 (只阻挡，不改变速度)
        // 注意：这里不检测石头
        checkObstacleCollisionForMount(p,PENGUIN_INITIAL_SPEED);
    }

    // --- 【新增】检测所有雪怪的碰撞 (逻辑完全相同) ---
    for (Yeti* y : m_yetis) {
        auto terrainInfo = getTerrainInfoAt(y->position().x());
        if (y->position().y() >= terrainInfo.first.y()) {
            y->onGround = true;
            QPointF pos = y->position();
            pos.setY(terrainInfo.first.y());
            y->setPosition(pos);
            y->setVelocity(QVector2D(y->velocity().x(), 0));
        } else {
            y->onGround = false;
        }
        checkObstacleCollisionForMount(y,YETI_INITIAL_SPEED);
    }

    // --- 阶段三：如果不在任何障碍物表面上，才检测与雪地的碰撞 ---
    if (!playerCollisionHandled) {
        auto terrainInfo = getTerrainInfoAt(m_player->position().x());
        qreal terrainY = terrainInfo.first.y();
        if (m_player->position().y() >= terrainY - 1.0 && m_player->velocity().y() >= 0) {
            isPlayerOnASurface = true;
            QPointF playerPos = m_player->position();
            playerPos.setY(terrainY);
            m_player->setPosition(playerPos);
            m_player->setRotation(terrainInfo.second);
            QVector2D airVelocity = m_player->velocity();
            QVector2D groundDirection(qCos(qDegreesToRadians(terrainInfo.second)), qSin(qDegreesToRadians(terrainInfo.second)));
            // qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
            // if (newSpeed < 0) newSpeed = 0;
            // m_player->setVelocity(groundDirection * newSpeed);
            m_player->setVelocity(groundDirection * m_player->currentSpeed());
            ///qDebug()<<"雪地上的速度"<<m_player->velocity();
            goto end_all_surface_checks;
        }
    }

end_all_surface_checks:
    // --- 阶段四：最终更新状态并检测与石头的碰撞 ---
    m_player->onGround = isPlayerOnASurface;

    for (Obstacle* obs : m_obstacles) {
        Stone* stone = dynamic_cast<Stone*>(obs);
        if (stone && stone->currentState == Stone::Intact) {
            if (m_player->collisionRect().intersects(stone->collisionRect())) {
                addScore(25);
                stone->shatter();
                m_player->crash(); // 调用摔倒函数
                //qDebug()<<"装在石头上"<<m_player->velocity();
            }
        }
    }

    // --- 【新增】阶段五：检测玩家与金币的碰撞 ---
    for (auto it = m_coins.begin(); it != m_coins.end(); ) {
        Coin* coin = *it;
        if (m_player->collisionRect().intersects(coin->collisionRect())) {

            AudioManager::instance()->playSoundEffect(SfxType::CoinGet);

            qDebug() << "金币被吃掉了！暂停游戏，准备弹窗...";
            stopGame(); // 2. 暂停游戏 (未来启用)
            m_cardDialog->generateRandomCards(); // 【修改】调用新的抽卡函数
            if (m_cardDialog->exec() == QDialog::Accepted) {

                CardData selectedCard = m_cardDialog->getSelectedCardData();

                // --- 【核心修改】直接调用卡片自带的效果函数 ---
                if (selectedCard.applyEffect) {
                    selectedCard.applyEffect(this); // 把 this (GameScreen*) 传进去
                }
            }

            startGame(); // 继续游戏

            // 4. 从游戏中移除这个金币
            it = m_coins.erase(it);
            coin->deleteLater();
            return; // 吃到一个金币后，立刻结束本帧的碰撞检测
        } else {
            ++it;
        }
    }
}

\


// --- 【最终修正版】一个功能强大的、可复用的坐骑碰撞函数 ---
template<typename MountType>
void GameScreen::checkObstacleCollisionForMount(MountType* mount, qreal initialSpeed) // <-- 【新增】传入初始速度
{
    // A. 先假设坐骑在空中
    mount->onGround = false;
    bool mountOnSurface = false;

    // --- B. 检测与障碍物表面的碰撞 (房屋, 翘板等) ---
    // (为了代码简洁，我们将房屋和翘板的检测合并)
    QList<QPainterPath> obstaclePaths;
    //for (House* house : m_houses) { obstaclePaths.append(house->getRoofPath()); }
    for (Seesaw* seesaw : m_seesaws) { obstaclePaths.append(seesaw->getPlankPath()); }
    // (告示牌的路径也可以在这里加入)

    for (const QPainterPath& path : obstaclePaths) {
        auto surfaceInfo = getPathInfoAt(path, mount->position().x());
        if (surfaceInfo.first.y() != 0 && mount->position().y() >= surfaceInfo.first.y() - 1.0 && mount->velocity().y() >= 0) {
            mountOnSurface = true;
            qreal angle = surfaceInfo.second;
            QVector2D dir(qCos(qDegreesToRadians(angle)), qSin(qDegreesToRadians(angle)));

            mount->setPosition(QPointF(mount->position().x(), surfaceInfo.first.y()));
            mount->setRotation(angle);
            // 【核心修复】速度大小被强制设为初始速度
            mount->setVelocity(dir * initialSpeed);
            goto end_mount_collision_check;
        }
    }

    // --- C. 如果没碰到任何障碍物，再检测与雪地的碰撞 ---
    if (!mountOnSurface) {
        auto terrainInfo = getTerrainInfoAt(mount->position().x());
        if (mount->position().y() >= terrainInfo.first.y() && mount->velocity().y() >= 0) {
            mount->onGround = true;
            QPointF pos = mount->position();
            pos.setY(terrainInfo.first.y());
            mount->setPosition(pos);

            qreal angle = terrainInfo.second;
            QVector2D dir(qCos(qDegreesToRadians(angle)), qSin(qDegreesToRadians(angle)));
            mount->setRotation(angle);
            // 【核心修复】在雪地上时，速度大小也被强制设为初始速度
            mount->setVelocity(dir * initialSpeed);
        } else {
            mount->onGround = false;
        }
    }

end_mount_collision_check:;
}





// --- 新增：触发器检测的实现 ---
void GameScreen::checkTriggers()
{
    if (!m_player) return;

    for (House* house : m_houses) {
        // 定义一个房子的触发区域 (例如，房子左侧的一个矩形区域)
        QRectF triggerZone = QRectF(house->position().x() - 100, house->position().y() - 200, 100, 200);

        // 如果玩家进入了这个区域
        if (triggerZone.contains(m_player->position())) {
            house->openDoor();
        }

        // 未来在这里还可以检测其他物体，如企鹅和雪怪
    }
}

// --- 【核心新增】实现通用的路径分析函数 ---
// 这个函数可以分析任何由直线段组成的 QPainterPath
QPair<QPointF, qreal> GameScreen::getPathInfoAt(const QPainterPath& path, qreal x_pos)
{
    // 遍历路径中的每一个元素
    for (int i = 0; i < path.elementCount() - 1; ++i) {
        const QPainterPath::Element& p1_element = path.elementAt(i);
        const QPainterPath::Element& p2_element = path.elementAt(i + 1);

        // 我们只关心直线段
        if (p1_element.isLineTo() || p1_element.isMoveTo()) {
            QPointF p1(p1_element.x, p1_element.y);
            QPointF p2(p2_element.x, p2_element.y);

            // 确保 p1 在 p2 的左边，便于计算
            if (p1.x() > p2.x()) {
                qSwap(p1, p2);
            }

            // 如果给定的 x_pos 在这个线段的水平范围内
            if (p1.x() <= x_pos && p2.x() >= x_pos) {
                qreal y_pos;
                // 使用线性插值计算精确的Y坐标
                if (p2.x() - p1.x() == 0) { // 垂直线段
                    y_pos = p1.y();
                } else {
                    qreal t = (x_pos - p1.x()) / (p2.x() - p1.x());
                    y_pos = p1.y() + t * (p2.y() - p1.y());
                }

                // 使用 atan2 计算坡度的角度
                qreal angle = qAtan2(p2.y() - p1.y(), p2.x() - p1.x()) * 180 / M_PI;

                return qMakePair(QPointF(x_pos, y_pos), angle);
            }
        }
    }
    // 如果没找到，返回一个无效值
    return qMakePair(QPointF(0, 0), 0.0);
}

// --- 【新增】计算速度与切面夹角的函数实现 ---
qreal GameScreen::calculateImpactAngle(const QVector2D& velocity, qreal surfaceAngleDegrees) const
{

    qreal velocityAngleDegrees = qAtan2(velocity.y(), velocity.x()) * 180 / M_PI;
    qreal angleDiff = fabs(velocityAngleDegrees - surfaceAngleDegrees);
    // qDebug()<<"角色矢量角度："<<velocityAngleDegrees;
    // qDebug()<<"障碍矢量角度："<<surfaceAngleDegrees;
    // 3. 将角度差归一化到 0-180 度之间，得到我们需要的“夹角”
    if (angleDiff > 180) {
        angleDiff = 360 - angleDiff;
    }

    return angleDiff;
}

// --- 【新增】平缓雪地生成函数 (长度约 2160) ---
void GameScreen::generateGentleSlope(QList<QPointF>& points, const QPointF& startPoint)
{
    const qreal length = 2160; // 宽度 1080 * 2
    const qreal drop = 200;    // 总下降高度 200
    const int segments = 108;  // 分段数 (2160 / 20)

    for (int i = 0; i <= segments; ++i) {
        qreal progress = (qreal)i / segments; // 进度: 0.0 -> 1.0
        qreal x = startPoint.x() + progress * length;

        // 使用 0.5 * (1 - cos(t * PI)) 的缓动曲线
        // 它的导数在起点和终点都为0，能确保与其它地形平滑连接
        qreal y_offset = 0.5 * (1 - cos(progress * M_PI)) * drop;
        points.append(QPointF(x, startPoint.y() + y_offset));
    }
}

// --- 【新增】陡峭雪地生成函数 (长度 1080) ---
void GameScreen::generateSteepSlope(QList<QPointF>& points, const QPointF& startPoint)
{
    QPointF currentPoint = startPoint;

    // --- 第一段：平地1 (用于生成翘板) ---
    const qreal flat1_length = 200;
    const qreal flat1_drop = 20; // 轻微下降
    const int flat1_segments = 10;
    for (int i = 0; i <= flat1_segments; ++i) {
        qreal progress = (qreal)i / flat1_segments;
        qreal x = currentPoint.x() + progress * flat1_length;
        qreal y_offset = 0.5 * (1 - cos(progress * M_PI)) * flat1_drop;
        points.append(QPointF(x, currentPoint.y() + y_offset));
    }
    currentPoint = points.last(); // 更新当前点

    // --- 第二段：陡坡 (45度) ---
    const qreal steep_length = 680;
    const qreal steep_drop = steep_length * 0.577; // 长度和高度差相等，即为45度
    const int steep_segments = 34;
    for (int i = 1; i <= steep_segments; ++i) { // 从1开始，避免重复添加点
        qreal progress = (qreal)i / steep_segments;
        qreal x = currentPoint.x() + progress * steep_length;
        // 使用 3t^2 - 2t^3 的缓动曲线，使得出入口平滑
        qreal y_offset = (3 * pow(progress, 2) - 2 * pow(progress, 3)) * steep_drop;
        points.append(QPointF(x, currentPoint.y() + y_offset));
    }
    currentPoint = points.last();

    // --- 第三段：平地2 (用于生成翘板) ---
    const qreal flat2_length = 200;
    const qreal flat2_drop = 20;
    const int flat2_segments = 10;
    for (int i = 1; i <= flat2_segments; ++i) {
        qreal progress = (qreal)i / flat2_segments;
        qreal x = currentPoint.x() + progress * flat2_length;
        qreal y_offset = 0.5 * (1 - cos(progress * M_PI)) * flat2_drop;
        points.append(QPointF(x, currentPoint.y() + y_offset));
    }
}

// --- 【最终优化版】悬崖生成函数 (45度，出口平滑) ---
void GameScreen::generateCliff(QList<QPointF>& points, const QPointF& startPoint)
{
    QPointF currentPoint = startPoint;

    // --- 第一段：起跳平台 (50px) ---
    // 这段保持不变，它的出口本身就是平滑的
    const qreal flat1_length = 50;
    const qreal flat1_drop = 5;
    const int flat1_segments = 3;
    for (int i = 0; i <= flat1_segments; ++i) {
        qreal progress = (qreal)i / flat1_segments;
        qreal x = currentPoint.x() + progress * flat1_length;
        qreal y_offset = 0.5 * (1 - cos(progress * M_PI)) * flat1_drop;
        points.append(QPointF(x, currentPoint.y() + y_offset));
    }
    currentPoint = points.last();

    // --- 第二段：悬崖陡坡 (440px) ---
    const qreal cliff_length = 440;
    const qreal cliff_drop = cliff_length * 2.0; // 45度角
    const int cliff_segments = 22;
    for (int i = 1; i <= cliff_segments; ++i) {
        qreal progress = (qreal)i / cliff_segments;
        qreal x = currentPoint.x() + progress * cliff_length;

        // 【核心修改】使用 3t^2 - 2t^3 的平滑曲线
        // 这条曲线能确保在 progress=0 和 progress=1 时，切线斜率都为0
        // 从而完美地与前后的平地对接
        qreal y_offset = (3 * pow(progress, 2) - 2 * pow(progress, 3)) * cliff_drop;
        points.append(QPointF(x, currentPoint.y() + y_offset));
    }
    currentPoint = points.last();

    // --- 第三段：落地平台 (50px) ---
    // 由于第二段的出口现在是平滑的，这段直接连接即可
    const qreal flat2_length = 50;
    const int flat2_segments = 3;
    for (int i = 1; i <= flat2_segments; ++i) {
        qreal x = currentPoint.x() + i * (flat2_length / flat2_segments);
        points.append(QPointF(x, currentPoint.y())); // Y坐标不变，保持水平
    }
}


// 在 src/ui/gamescreen.cpp 的文件末尾

void GameScreen::applyScoreMultiplier(float multiplier, int duration)
{
    // 如果当前已经有一个更高倍率的效果正在持续，则不应用新的低倍率效果
    if (m_scoreMultiplierTimer->isActive() && multiplier <= m_scoreMultiplier) {
        qDebug() << "已存在更高或相同的得分倍率效果，忽略本次请求。";
        return;
    }

    qDebug() << "得分倍率提升！" << multiplier << "x，持续" << duration << "毫秒。";
    m_scoreMultiplier = multiplier;

    // 停止上一个计时器（如果有的话），并用新的持续时间启动它
    m_scoreMultiplierTimer->start(duration);
}


// 在 src/ui/gamescreen.cpp 的文件末尾添加

void GameScreen::showDebugInfo()
{
    // 1. 暂停游戏
    stopGame();

    // 2. 创建一个模态对话框
    m_debugDialog = new QDialog(this);
    m_debugDialog->setWindowTitle("实时调试信息");
    m_debugDialog->setFixedSize(400, 300);

    // 3. 创建用于显示信息的标签
    QLabel* infoContent = new QLabel(m_debugDialog);
    infoContent->setWordWrap(true);
    infoContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoContent->setStyleSheet("font-size: 14px; padding: 10px;");

    // 4. 准备要显示的文本内容
    QString debugText;
    if (m_player) {
        QString gravityStr = QString::number(m_player->currentGravity(), 'f', 2);
        QString jumpForceStr = QString::number(m_player->jumpForce(), 'f', 2);
        QString mountInfoText = "无";

        if(m_player->isMounted()) {
            mountInfoText = (m_player->currentMountType() == Player::Penguin) ? "企鹅" : "雪怪/损坏的雪怪";
        }

        debugText += QString("<b>角色状态:</b><br>");
        debugText += QString("  - 当前坐骑: %1<br>").arg(mountInfoText);
        debugText += QString("  - 重力加速度: %1<br>").arg(gravityStr);
        debugText += QString("  - 弹跳力: %1<br><br>").arg(jumpForceStr);
    }

    debugText += QString("<b>游戏参数:</b><br>");
    debugText += QString("  - 企鹅速度倍率: x%1<br>").arg(PLAYER_SPEED_MULTIPLIER_ON_PENGUIN);
    debugText += QString("  - 雪怪速度倍率: x%1<br>").arg(PLAYER_SPEED_MULTIPLIER_ON_YETI);
    debugText += QString("  - 当前分数倍率: x%1<br>").arg(QString::number(m_scoreMultiplier, 'f', 1));

    infoContent->setText(debugText);

    // 5. 设置对话框布局
    QVBoxLayout* layout = new QVBoxLayout(m_debugDialog);
    layout->addWidget(infoContent);
    m_debugDialog->setLayout(layout);

    // 6. 显示对话框，这会阻塞游戏循环直到对话框被关闭
    m_debugDialog->exec();

    // 7. 对话框关闭后，清理并继续游戏
    delete m_debugDialog;
    m_debugDialog = nullptr;
    startGame();
    this->setFocus(); // 确保焦点回到游戏窗口
}
