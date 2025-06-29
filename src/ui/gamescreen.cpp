#include "ui/GameScreen.h"
#include "game/Player.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <QtMath> // 用于计算角度
#include <QVector3D> // <-- 在这里添加这一行
#include<QKeyEvent>
#include "game/Stone.h"
#include "game/House.h" // <-- 引入 House 头文件
#include "game/Seesaw.h"
#include "game/Signboard.h"
#include "ui/ImageButton.h" // <-- 【新增】
#include "ui/PauseDialog.h" // <-- 【新增】
#include "game/Avalanche.h"
#include "ui/GameOverDialog.h"
// 定义地形的一些常量，便于调整
const int TERRAIN_POINT_INTERVAL = 20; // 每个地形点的水平间距
const int TERRAIN_MIN_Y = 450;         // 地形的最低高度
const int TERRAIN_MAX_Y = 550;         // 地形的最高高度
const qreal SCROLL_SPEED = 2.0;        // 地面的滚动速度
const qreal PLAYER_VISIBLE_Y_MIN = 350.0;
const qreal PLAYER_VISIBLE_Y_MAX = 400.0;
// 定义玩家在屏幕上固定的X轴位置
const qreal PLAYER_FIXED_X = 200.0;
const qreal initialSpeed = 12.5; // 初始速度大小
GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent), m_backgroundOffset(0),
    m_worldOffset(0), m_verticalOffset(0) // 初始化 m_worldOffset
{
    // 1. 加载背景图，如果失败则创建一个蓝色背景作为占位符
    if (!m_backgroundPixmap.load(":/assets/images/game_background.png")) {
        qWarning("Could not load game background image, creating a placeholder.");
        // 您期望的窗口尺寸是 1080x720，但主窗口目前是 800x600
        // 这里我们先使用主窗口的尺寸，便于查看
        m_backgroundPixmap = QPixmap(800, 600);
        m_backgroundPixmap.fill(QColor("#87CEEB")); // 天蓝色
    }

    setupUI();
    // 2. 创建并连接计时器，用于驱动游戏循环
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameScreen::updateGame);
    connect(m_timer, &QTimer::timeout, this, &GameScreen::updateSnow);

    // 3. 初始化地形
    generateInitialTerrain();

    // 4. 创建并放置玩家
    resetGameState();

    setFocusPolicy(Qt::StrongFocus); // 确保 GameScreen 能接收键盘事件

    // --- 新增：在构造时调用，放置所有障碍物 ---
    setupObstacles();
}
// --- 【新增】UI设置函数的实现 ---
void GameScreen::setupUI()
{
    // 1. 创建暂停按钮
    m_pauseButton = new ImageButton(":/assets/images/btn_pause.png", this);
    // 将其移动到右上角，留出一些边距
    //m_pauseButton->move(width() - m_pauseButton->width() - 20, 20);
    m_pauseButton->move(1200, 20);

    // 2. 创建暂停对话框
    m_pauseDialog = new PauseDialog(this);
    m_pauseDialog->hide(); // 默认隐藏

    // --- 【新增】创建游戏结束对话框 ---
    m_gameOverDialog = new GameOverDialog(this);
    m_gameOverDialog->hide(); // 默认隐藏

    // 3. 连接所有信号与槽
    connect(m_pauseButton, &QPushButton::clicked, this, &GameScreen::onPauseButtonClicked);
    connect(m_pauseDialog, &PauseDialog::resumeClicked, this, &GameScreen::startGame); // 继续游戏就是重启计时器
    connect(m_pauseDialog, &PauseDialog::restartClicked, this, &GameScreen::restartGame);
    // 【修改】将暂停菜单的退出按钮连接到新的 handleExit 槽
    connect(m_pauseDialog, &PauseDialog::exitClicked, this, &GameScreen::handleExit);

    connect(m_gameOverDialog, &GameOverDialog::restartClicked, this, &GameScreen::restartGame);
    // 【修改】将游戏结束菜单的退出按钮也连接到新的 handleExit 槽
    connect(m_gameOverDialog, &GameOverDialog::exitClicked, this, &GameScreen::handleExit);
}
// --- 【新增】暂停按钮的槽函数实现 ---
void GameScreen::onPauseButtonClicked()
{
    stopGame(); // 停止游戏计时器
    m_pauseDialog->exec(); // 以模态方式显示暂停对话框，会阻塞游戏窗口
    // 【新增】对话框关闭后，立刻将焦点还给游戏窗口
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
// --- 【新增】重启游戏的槽函数实现 ---
// void GameScreen::restartGame()
// {
//     // 这是一个简化的重启逻辑，后续可以扩展
//     // 1. 清理旧的对象
//     m_snowPoints.clear();
//     qDeleteAll(m_obstacles);
//     m_obstacles.clear();
//     qDeleteAll(m_houses);
//     m_houses.clear();
//     qDeleteAll(m_seesaws);
//     m_seesaws.clear();
//     qDeleteAll(m_signboards);
//     m_signboards.clear();
//     delete m_player;
//     m_player = nullptr;

//     // --- 【新增】清理旧的雪崩对象 ---
//     delete m_avalanche;
//     m_avalanche = nullptr;

//     // --- 【新增】如果游戏结束对话框是打开的，也关掉它 ---
//     if (m_gameOverDialog->isVisible()) {
//         m_gameOverDialog->accept();
//     }

//     // 2. 重置世界和镜头偏移
//     m_worldOffset = 0;
//     m_verticalOffset = 0;

//     // 3. 重新创建游戏世界
//     generateInitialTerrain();
//     resetGameState();
//     setupObstacles();

//     // 4. 关闭对话框并开始游戏
//     m_pauseDialog->accept();
//     startGame();
//     // 【新增】游戏开始后，立刻将焦点还给游戏窗口
//     this->setFocus();
// }

// 【修改后】一个功能更强大的、公开的 restartGame 函数
void GameScreen::restartGame()
{
    // 1. 确保所有计时器都已停止
    stopGame();

    // 2. 如果有对话框是打开的，先关闭它们
    if (m_pauseDialog->isVisible()) {
        m_pauseDialog->accept();
    }
    if (m_gameOverDialog->isVisible()) {
        m_gameOverDialog->accept();
    }

    // 3. 清理所有游戏对象，回到“空无一物”的状态
    m_snowPoints.clear();
    qDeleteAll(m_obstacles);
    m_obstacles.clear();
    qDeleteAll(m_houses);
    m_houses.clear();
    qDeleteAll(m_seesaws);
    m_seesaws.clear();
    qDeleteAll(m_signboards);
    m_signboards.clear();
    delete m_player;
    m_player = nullptr;
    delete m_avalanche;
    m_avalanche = nullptr;

    // 4. 重置世界和镜头的偏移量
    m_worldOffset = 0;
    m_verticalOffset = 0;

    // 5. 像第一次启动一样，重新创建整个游戏世界
    generateInitialTerrain();
    resetGameState();
    setupObstacles();

    // 6. 重新开始游戏循环
    startGame();

    // 7. 确保游戏窗口获得键盘焦点
    setFocus();
}

void GameScreen::startGame()
{
    // 每 16 毫秒更新一次，约等于 60 FPS
    m_timer->start(16);
}

void GameScreen::stopGame()
{
    m_timer->stop();
}

void GameScreen::updateGame()
{
    //背景滚动逻辑
    m_backgroundOffset -= 0.5; // 让背景滚动得比地面慢，产生视差效果
    // 2. 当背景完全移出左边界时，将其向右移动一个宽度的距离，实现无缝循环
    //    而不是跳回0！
    if (m_backgroundOffset <= -width()) {
        m_backgroundOffset += width();
    }

    // --- 玩家状态更新 (核心修改) ---
    if(m_player) {
        // 1. 调用玩家自己的更新，这会根据当前速度改变位置
        m_player->update();

        // 2. 获取玩家脚下地形的信息
        // QPointF playerPos = m_player->position();
        // auto terrainInfo = getTerrainInfoAt(playerPos.x());
        // qreal terrainY = terrainInfo.first.y();
        // qreal terrainAngle = terrainInfo.second;

        // // // 3. 让玩家“粘”在地面上
        // // //    这里是一个简化的实现，直接将玩家的Y坐标设置为地形的高度。
        // // //    后续可以加入重力，当玩家在空中时，此逻辑不生效。
        // // playerPos.setY(terrainY);
        // // m_player->setPosition(playerPos);

        // // // 4. 根据脚下的坡度，实时更新玩家的旋转角度和速度方向
        // // m_player->setRotation(terrainAngle);

        // // // 获取当前速度大小
        // // qreal currentSpeed = m_player->velocity().length();
        // // // 将速度大小和新的坡度角度结合，计算出新的速度矢量
        // // qreal angleRad = qDegreesToRadians(terrainAngle);
        // // QVector2D newVelocity(currentSpeed * qCos(angleRad), currentSpeed * qSin(angleRad));
        // // m_player->setVelocity(newVelocity);

        // // 3. 【核心】地面检测与处理
        // if (playerPos.y() >= terrainY && m_player->velocity().y() >= 0) {
        //     // 条件：玩家位置在地面下方 或 与地面平齐，并且玩家正在下落（或水平移动）
        //     // 意味着玩家“着陆”了

        //     // 将玩家精确地放回地面
        //     playerPos.setY(terrainY);
        //     m_player->setPosition(playerPos);

        //     // 标记玩家在地面上
        //     m_player->onGround = true;

        //     // 根据脚下的坡度，调整玩家的旋转和速度
        //     m_player->setRotation(terrainAngle);

        //     // // 获取当前速度大小，并根据新坡度重新计算速度矢量
        //     // // 这使得玩家着陆后能顺着斜坡滑行
        //     // qreal currentSpeed = m_player->velocity().length();
        //     // qreal angleRad = qDegreesToRadians(terrainAngle);
        //     // QVector2D newVelocity(currentSpeed * qCos(angleRad), currentSpeed * qSin(angleRad));

        //     // // 当玩家在地面上时，我们不再关心垂直速度，所以可以只保留水平速度，或者直接使用新计算的速度
        //     // m_player->setVelocity(newVelocity);
        //     // 1. 获取着陆前的空中速度矢量
        //     QVector2D airVelocity = m_player->velocity();

        //     // 2. 计算出地面坡度的单位方向向量
        //     qreal angleRad = qDegreesToRadians(terrainAngle);
        //     QVector2D groundDirection(qCos(angleRad), qSin(angleRad));

        //     // 3. 计算空中速度在地面方向上的投影（点积）
        //     qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);

        //     // 安全检查：如果玩家几乎是逆着山坡撞上去，速度可能为负，我们不希望他倒滑，所以取0
        //     if (newSpeed < 0) {
        //         newSpeed = 0;
        //     }

        //     // 4. 用这个新的速度大小和地面方向来生成最终的速度矢量
        //     QVector2D newVelocity = groundDirection * newSpeed;
        //     m_player->setVelocity(newVelocity);


        // } else {
        //     // 如果不在上述条件内，说明玩家在空中
        //     m_player->onGround = false;
        // }

        // --- 3. 【核心修改】更新世界偏移量 ---
        // 世界的偏移量 = 玩家在世界中的X坐标 - 玩家在屏幕上固定的X坐标
        m_worldOffset = m_player->position().x() - PLAYER_FIXED_X;
        // 2. 更新垂直镜头偏移 (新增逻辑)
        // 定义一个角色在屏幕上活动的“安全窗口”


        // 计算角色当前在屏幕上的Y坐标
        qreal playerScreenY = m_player->position().y() - m_verticalOffset;

        if (playerScreenY > PLAYER_VISIBLE_Y_MAX) {
            // 如果角色低于窗口下沿，则镜头向下移动
            m_verticalOffset = m_player->position().y() - PLAYER_VISIBLE_Y_MAX;
        } else if (playerScreenY < PLAYER_VISIBLE_Y_MIN) {
            // 如果角色高于窗口上沿，则镜头向上移动
            m_verticalOffset = m_player->position().y() - PLAYER_VISIBLE_Y_MIN;
        }
        // 【核心修正】让背景图以一个恒定的慢速独立滚动
        // m_backgroundOffset += 1.5; // 您可以调整这个值来控制背景滚动的快慢
        // if (m_backgroundOffset >= width()) { // 相应地，这里的判断条件也要调整
        //     m_backgroundOffset -= width();
        // }
        checkCollisions();
    }

    // --- 【新增】更新雪崩的位置 ---
    if(m_avalanche) {
        m_avalanche->update();
    }

    // --- 【新增】检查游戏是否结束 ---
    if (m_avalanche && m_player) {
        // 计算雪崩右侧的X坐标
        qreal avalancheRightEdge = m_avalanche->position().x() + m_avalanche->width() / 2.0;
        // 判断雪崩的右侧是否触碰到玩家的X坐标（可以根据玩家的实际碰撞范围进行微调）
        if (avalancheRightEdge >= m_player->position().x()+30) {
            stopGame(); // 停止游戏
            m_gameOverDialog->exec(); // 显示游戏结束对话框
            return; // 立刻返回，不再执行后续更新
        }
    }


    // --- 新增：每帧都检查碰撞 ---

    checkTriggers(); // <-- 新增调用
    // --- 新增：更新所有障碍物的状态 ---
    // (这对于播放破碎动画很重要)
    for (Obstacle* obs : m_obstacles) {
        obs->update();
    }

    // 更新雪地地形
    updateSnow();



    // 请求重新绘制界面
    update();

    if(m_player) {
        m_worldOffset = m_player->position().x() - PLAYER_FIXED_X;
    }

}

// --- 新增：实现键盘事件处理函数 ---
void GameScreen::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return; // 忽略长按的重复事件
    }

    if (m_player) {
        switch (event->key()) {
        case Qt::Key_Space: // 如果按下的是空格键
            m_player->jump();
            break;
        // 在这里可以添加其他按键控制，如加速、使用道具等
        default:
            QWidget::keyPressEvent(event); // 其他按键交给父类处理
        }
    }
}
// int i = 0;
// void GameScreen::updateSnow()
// {
//     // --- 【核心修改】现在雪地的生成和销毁要基于摄像机的位置 ---
//     // 移除已经完全移出摄像机左侧的点
//     while (m_snowPoints.size() > 2 && m_snowPoints[1].x() < m_worldOffset) {
//         m_snowPoints.removeFirst();
//     }

//     // 在摄像机右侧补充新的点

//     while (m_snowPoints.last().x() < m_worldOffset + width() + TERRAIN_POINT_INTERVAL) {
//         qreal lastX = m_snowPoints.last().x();
//         qreal lastY = m_snowPoints.last().y();
//         //此处可以加入更丰富的地形生成算法
//         // qreal newY = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
//         // newY = (lastY * 3 + newY) / 4; // 做一个简单的平滑处理
//         qreal newY = lastY;
//         if(i>100){
//             i = 0;
//         }
//         if(i<=50){
//             newY += i/10;
//         }else{
//             //newY -= (i-50)/10+0.3;
//         }
//         i++;


//         m_snowPoints.append(QPointF(lastX + TERRAIN_POINT_INTERVAL, newY));

//     }

//     // 根据更新后的点，重新生成可绘制的路径
//     updateSnowPath();


//     // // 1. 将所有地形点的X坐标向左移动，实现滚动
//     // for (int i = 0; i < m_snowPoints.size(); ++i) {
//     //     m_snowPoints[i].setX(m_snowPoints[i].x() - SCROLL_SPEED);
//     // }

//     // // 2. 移除已经完全移出屏幕左侧的点
//     // // 我们多保留一个点，确保曲线在屏幕外依然平滑
//     // if (!m_snowPoints.isEmpty() && m_snowPoints.first().x() < -TERRAIN_POINT_INTERVAL) {
//     //     m_snowPoints.removeFirst();
//     // }

//     // // 3. 在右侧补充新的点，以创建无限地形
//     // while (!m_snowPoints.isEmpty() && m_snowPoints.last().x() < width()) {
//     //     qreal lastX = m_snowPoints.last().x();
//     //     qreal lastY = m_snowPoints.last().y();

//     //     // 生成一个稍微有点起伏的新Y坐标
//     //     //qreal newY = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
//     //     // 为了平滑，可以取上一个点和新随机点的平均值
//     //     //newY = (lastY + newY) / 2;

//     //     //测试平地
//     //     qreal newY = lastY;

//     //     m_snowPoints.append(QPointF(lastX + TERRAIN_POINT_INTERVAL, newY));
//     // }

//     // // 4. 根据更新后的点，重新生成可绘制的路径
//     // updateSnowPath();
// }


// 用以下代码完整替换现有的 updateSnow() 函数
void GameScreen::updateSnow()
{
    // --- 移除已经完全移出摄像机左侧的点 ---
    while (m_snowPoints.size() > 2 && m_snowPoints[1].x() < m_worldOffset) {
        m_snowPoints.removeFirst();
    }

    // --- 检查是否需要生成新的地形块 ---
    // 如果“弹夹”是空的，并且当前地形即将离开屏幕右侧
    if (m_nextTerrainSegment.isEmpty() && m_snowPoints.last().x() < m_worldOffset + width() + 500) {

        // 1. 使用概率系统决定下一个地形的类型
        double choice = QRandomGenerator::global()->generateDouble();

        if (choice < 0.02) { // 2% 的概率
            generateCliff(m_nextTerrainSegment, m_snowPoints.last());
        } else if (choice < 0.30) { // 28% 的概率 (0.02 + 0.28)
            generateSteepSlope(m_nextTerrainSegment, m_snowPoints.last());
        } else { // 70% 的概率
            generateGentleSlope(m_nextTerrainSegment, m_snowPoints.last());
        }
    }

    // --- 如果“弹夹”里有货，并且屏幕右侧需要补充地形，就把“弹夹”里的点加进去 ---
    if (!m_nextTerrainSegment.isEmpty() && m_snowPoints.last().x() < m_worldOffset + width() + TERRAIN_POINT_INTERVAL) {
        m_snowPoints.append(m_nextTerrainSegment);
        m_nextTerrainSegment.clear(); // 清空“弹夹”
    }

    // 根据更新后的点，重新生成可绘制的路径
    updateSnowPath();
}


void GameScreen::generateInitialTerrain()
{
    // // 清空旧数据
    // m_snowPoints.clear();

    // // 生成从屏幕左侧到右侧，并额外多一个点的地形数据
    // for (int x = 0; x <= width() + TERRAIN_POINT_INTERVAL; x += TERRAIN_POINT_INTERVAL) {
    //     //int y = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
    //     //测试平地
    //     int y = 500;
    //     m_snowPoints.append(QPointF(x, y));
    // }

    // // 根据初始点生成路径
    // updateSnowPath();
    m_snowPoints.clear();
    m_nextTerrainSegment.clear(); // 确保“弹夹”是空的

    // 游戏开始时，总是先生成一段平缓的雪地
    generateGentleSlope(m_snowPoints, QPointF(0, 500));
    updateSnowPath();
}

void GameScreen::updateSnowPath()
{
    if (m_snowPoints.size() < 2) return;

    // 开始构建路径
    m_snowPath = QPainterPath();
    // 【核心修正】计算当前镜头的实际底部Y坐标
    qreal bottomY = m_verticalOffset + height();
    // 1. 将起点移动到屏幕左下角 (使用新的底部坐标)
    m_snowPath.moveTo(m_snowPoints.first().x(), bottomY);
    // 2. 画一条线到第一个地形点
    m_snowPath.lineTo(m_snowPoints.first());
    // 3. 将所有地形点连接成一条曲线
    for (int i = 1; i < m_snowPoints.size(); ++i) {
        m_snowPath.lineTo(m_snowPoints[i]);
    }
    // 4. 从最后一个地形点画一条线到屏幕右下角 (使用新的底部坐标)
    m_snowPath.lineTo(m_snowPoints.last().x(), bottomY);
    // 5. 闭合路径，形成一个封闭的多边形
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

    // --- 设置初始速度 (代码简化和优化) ---

    qreal initialAngleRad = qDegreesToRadians(m_player->rotation()); // 将初始角度转为弧度

    // 使用简单的2D三角函数计算速度矢量的x和y分量
    QVector2D initial_velocity(initialSpeed * qCos(initialAngleRad),
                               initialSpeed * qSin(initialAngleRad));

    m_player->setVelocity(initial_velocity);
    m_avalanche = new Avalanche(this, this);
    // 设置雪崩的初始位置在玩家身后较远的地方
    m_avalanche->setPosition(QPointF(m_player->position().x() - 800, m_player->position().y()));
    // 设置一个比玩家稍慢的初始速度
    m_avalanche->setVelocity(QVector2D(initial_velocity.x() * 0.8, 0));
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
    // 背景的偏移与世界偏移独立，以创造深度感
    // qreal bg_x = fmod(-m_backgroundOffset, width());
    //qreal bg_x = -m_backgroundOffset; // 偏移量直接为负，代表向左移动
    // 强制转换为整数，以消除浮点数精度导致的接缝问题
    int bg_x = static_cast<int>(m_backgroundOffset);
    // painter.drawPixmap(bg_x - width(), 0, width(), height(), m_backgroundPixmap);
    // painter.drawPixmap(bg_x, 0, width(), height(), m_backgroundPixmap);
    // painter.drawPixmap(bg_x + width(), 0, width(), height(), m_backgroundPixmap);
    // 绘制第一张背景图
    painter.drawPixmap(bg_x, 0, width(), height(), m_backgroundPixmap);

    // 绘制第二张背景图，紧跟在第一张的右边，实现无缝拼接
    painter.drawPixmap(bg_x + width(), 0, width(), height(), m_backgroundPixmap);

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

    for (Seesaw* seesaw : m_seesaws) {
        seesaw->draw(&painter);
    }

    for (Signboard* signboard : m_signboards) {
        signboard->draw(&painter);
    }
    // 3. 在这里绘制未来的企鹅、雪怪等...
    //   (它们也会被自动正确地放置在滚动的世界中)

    // --- 【新增】绘制雪崩 ---
    if(m_avalanche) {
        m_avalanche->draw(&painter);
    }


    painter.restore(); // 恢复坐标系，不影响后续的UI绘制

    // --- 在这里可以绘制固定的UI元素，如分数、生命值等 ---
    // painter.drawText(20, 40, "Score: ...");

    // Q_UNUSED(event);
    // QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，让曲线更平滑

    // // 1. 绘制滚动的背景
    // qreal x1 = m_backgroundOffset;
    // qreal x2 = m_backgroundOffset + width();
    // painter.drawPixmap(QRectF(x1, 0, width(), height()), m_backgroundPixmap, m_backgroundPixmap.rect());
    // painter.drawPixmap(QRectF(x2, 0, width(), height()), m_backgroundPixmap, m_backgroundPixmap.rect());

    // // 2. 绘制程序化生成的雪地
    // painter.setBrush(Qt::white); // 设置填充颜色为白色
    // painter.setPen(Qt::NoPen);   // 我们不需要边框线
    // painter.drawPath(m_snowPath);

    // // 3. 绘制玩家
    // if(m_player) {
    //     m_player->draw(&painter);
    // }
}

// --- 新增：放置障碍物的实现 ---
void GameScreen::setupObstacles()
{
    // 在这里创建并放置石头
    // 石头的坐标是基于世界坐标系
    Stone* stone1 = new Stone(Stone::Small, this);
    stone1->setPosition(QPointF(800, getTerrainInfoAt(800).first.y()));
    m_obstacles.append(stone1);

    Stone* stone2 = new Stone(Stone::Large, this);
    stone2->setPosition(QPointF(1500, getTerrainInfoAt(1500).first.y()));
    m_obstacles.append(stone2);


    // --- 新增：创建房屋 ---
    House* house1 = new House(this);
    // 房屋的Y坐标应该根据地形来设置，使其底部贴着地面
    house1->setPosition(QPointF(2000, getTerrainInfoAt(600).first.y()/*+55*/));
    m_houses.append(house1);


    // --- 【新增】创建翘板 ---
    Seesaw* seesaw1 = new Seesaw(this);
    // 将其放置在世界坐标 (2500, y) 并旋转 -15 度
    seesaw1->setPosition(QPointF(2500, getTerrainInfoAt(2500).first.y() ));
    //seesaw1->setRotation(-15.0); // 负数代表右上翘
    m_seesaws.append(seesaw1);

    // ... 在 setupObstacles() 函数末尾
    // --- 【新增】创建告示牌 ---
    Signboard* sign1 = new Signboard(this);
    sign1->setPosition(QPointF(3000, getTerrainInfoAt(3000).first.y()));
    //sign1->setRotation(-5.0); // 轻微倾斜
    m_signboards.append(sign1);
    // 您可以根据需要添加更多的石头或其他障碍物
}

// --- 【最终修正版】碰撞检测实现 (无 GOTO) ---
void GameScreen::checkCollisions()
{
    if (!m_player) return;

    bool isPlayerOnASurface = false;

    //--- 阶段一：检测与房屋的多种交互 ---
    for (House* house : m_houses) {
        // A. 检测是否撞到屋顶天花板 (从下往上)
        QPointF leftCorner = house->getRoofLeftCornerWorldPosition();
        QPointF rightCorner = house->getRoofRightCornerWorldPosition();
        if (m_player->velocity().y() < 0 &&
            m_player->position().x() > leftCorner.x() &&
            m_player->position().x() < rightCorner.x() &&
            m_player->collisionRect().top() < leftCorner.y()) {

            // 撞到天花板，这是硬碰撞
            QPointF playerPos = m_player->position();
            playerPos.setY(leftCorner.y() + m_player->collisionRect().height());
            m_player->setPosition(playerPos);
            m_player->setVelocity(QVector2D(m_player->velocity().x(), 0));
            house->shatter(QPointF(m_player->position().x(), leftCorner.y()));

            // 既然是硬碰撞，本帧的表面检测就可以结束了
            goto end_all_surface_checks; // 使用 goto 是为了立即跳出所有循环
        }

        QPainterPath roofPath = house->getRoofPath();
        auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
        if (roofInfo.first.y() != 0 && fabs(m_player->position().y() - roofInfo.first.y()) < 5.0) {

            house->shatter(m_player->position());
            qreal roofY = roofInfo.first.y();
            QPointF peakWorldPos = house->getRoofPeakWorldPosition();
            if (m_player->position().x() < peakWorldPos.x()) {
                qreal roofAngle = roofInfo.second;

                // fabs() 取绝对值，这样无论是上坡还是下坡，我们只关心坡度陡峭程度
                if (fabs(roofAngle) > 900.0) {
                    // 【逻辑 > 60度】: 角度过陡，执行摔倒（暂时穿模）
                    // 这里我们只校正位置防止卡住，不改变速度，让角色按原轨迹穿过去
                    QPointF playerPos = m_player->position();
                    playerPos.setY(roofInfo.first.y());
                    m_player->setPosition(playerPos);
                    // isPlayerOnASurface 保持 false，使其继续受重力影响
                } else {
                    // 【逻辑 <= 60度】: 正常滑行，保留速度，改变方向
                    isPlayerOnASurface = true;
                    QPointF playerPos = m_player->position();
                    playerPos.setY(roofInfo.first.y());
                    m_player->setPosition(playerPos);
                    m_player->setRotation(roofAngle);

                    // 获取当前的速度大小
                    qreal currentSpeed = m_player->velocity().length();
                    // 创建与斜坡平行的方向向量
                    QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
                    // 应用新的速度：方向为斜坡方向，大小为碰撞前的速度
                    m_player->setVelocity(groundDirection * currentSpeed);
                }
            } else {
                // 右侧挤压
                QPointF playerPos = m_player->position();
                playerPos.setY(roofInfo.first.y());
                m_player->setPosition(playerPos);
                goto end_all_surface_checks;
            }
            // 找到一个表面就跳出所有表面检测
            goto end_all_surface_checks;
        }
    }

    // for (House* house : m_houses) {
    //     // A. 检测天花板碰撞 (从下往上)
    //     QPointF leftCorner = house->getRoofLeftCornerWorldPosition();
    //     QPointF rightCorner = house->getRoofRightCornerWorldPosition();
    //     if (m_player->velocity().y() < 0 &&
    //         m_player->position().x() > leftCorner.x() &&
    //         m_player->position().x() < rightCorner.x() &&
    //         m_player->collisionRect().top() < leftCorner.y()) {

    //         // 撞到天花板，硬碰撞
    //         QPointF playerPos = m_player->position();
    //         playerPos.setY(leftCorner.y() + m_player->collisionRect().height());
    //         m_player->setPosition(playerPos);
    //         m_player->setVelocity(QVector2D(m_player->velocity().x(), 0));
    //         house->shatter(QPointF(m_player->position().x(), leftCorner.y()));
    //         goto end_all_surface_checks; // 发生硬碰撞，结束所有表面检测
    //     }

    //     // B. 检测屋顶表面碰撞 (从上往下)
    //     QPainterPath roofPath = house->getRoofPath();
    //     auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
    //     if (roofInfo.first.y() != 0 && fabs(m_player->position().y() - roofInfo.first.y()) < 5.0) {

    //         house->shatter(m_player->position());

    //         QPointF peakWorldPos = house->getRoofPeakWorldPosition();
    //         if (m_player->position().x() < peakWorldPos.x()) {
    //             // 左侧滑行
    //             isPlayerOnASurface = true;
    //             QPointF playerPos = m_player->position();
    //             playerPos.setY(roofInfo.first.y());
    //             m_player->setPosition(playerPos);
    //             m_player->setRotation(roofInfo.second);
    //             QVector2D airVelocity = m_player->velocity();
    //             QVector2D groundDirection(qCos(qDegreesToRadians(roofInfo.second)), qSin(qDegreesToRadians(roofInfo.second)));
    //             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
    //             if (newSpeed < 0) newSpeed = 0;
    //             m_player->setVelocity(groundDirection * newSpeed);
    //         } else {
    //             // 右侧挤压
    //             QPointF playerPos = m_player->position();
    //             playerPos.setY(roofInfo.first.y());
    //             m_player->setPosition(playerPos);
    //         }
    //         // 【重要】无论左右，只要在屋顶表面，就结束所有表面检测
    //         goto end_all_surface_checks;
    //     }
    // }

    // --- 阶段二：如果不在房屋上，再检测与翘板的碰撞 ---
    for (Seesaw* seesaw : m_seesaws) {
        if (seesaw->currentState() == Seesaw::Shattered) continue;

        QPainterPath plankPath = seesaw->getPlankPath();
        auto plankInfo = getPathInfoAt(plankPath, m_player->position().x());
        if (plankInfo.first.y() != 0 && fabs(m_player->position().y() - plankInfo.first.y()) < 5.0) {

            qreal plankAngle = plankInfo.second;

            if (fabs(plankAngle) > 900.0) {
                // 【逻辑 > 60度】: 角度过陡，摔倒（暂时穿模）
                QPointF playerPos = m_player->position();
                playerPos.setY(plankInfo.first.y());
                m_player->setPosition(playerPos);
            } else {
                // 【逻辑 <= 60度】: 正常滑行
                isPlayerOnASurface = true;
                QPointF playerPos = m_player->position();
                playerPos.setY(plankInfo.first.y());
                m_player->setPosition(playerPos);
                m_player->setRotation(plankAngle);

                qreal currentSpeed = m_player->velocity().length();
                QVector2D groundDirection(qCos(qDegreesToRadians(plankAngle)), qSin(qDegreesToRadians(plankAngle)));
                m_player->setVelocity(groundDirection * currentSpeed);
            }

            goto end_all_surface_checks;
        }
    }

    for (Signboard* sign : m_signboards) {
        if (sign->currentState() == Signboard::Shattered) continue;

        QPainterPath boardPath = sign->getBoardPath();
        auto boardInfo = getPathInfoAt(boardPath, m_player->position().x());
        if (boardInfo.first.y() != 0 && fabs(m_player->position().y() - boardInfo.first.y()) < 5.0) {

            sign->shatter(m_player->position());

            // 【完全复刻的逻辑】根据角度判断滑行或摔倒
            qreal boardAngle = boardInfo.second;
            if (fabs(boardAngle) > 80.0) {
                QPointF playerPos = m_player->position();
                playerPos.setY(boardInfo.first.y());
                m_player->setPosition(playerPos);
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

            goto end_all_surface_checks;
        }
    }

    // --- 阶段三：如果不在任何障碍物表面上，才检测与雪地的碰撞 ---
    {
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
            qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
            if (newSpeed < 0) newSpeed = 0;
            m_player->setVelocity(groundDirection * newSpeed);
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
                stone->shatter();
            }
        }
    }
}


// --- 【最终修正版】碰撞检测实现 ---
// void GameScreen::checkCollisions()
// {
//     if (!m_player) return;

//     bool isPlayerOnASurface = false;

//     // --- 阶段一：检测硬碰撞 (天花板) ---
//     for (House* house : m_houses) {
//         QPointF leftCorner = house->getRoofLeftCornerWorldPosition();
//         QPointF rightCorner = house->getRoofRightCornerWorldPosition();
//         qreal playerTopY = m_player->collisionRect().top();

//         if (m_player->velocity().y() < 0 &&
//             m_player->position().x() > leftCorner.x() &&
//             m_player->position().x() < rightCorner.x() &&
//             playerTopY < leftCorner.y()) {

//             // 撞到天花板
//             QPointF playerPos = m_player->position();
//             playerPos.setY(leftCorner.y() + m_player->collisionRect().height());
//             m_player->setPosition(playerPos);
//             m_player->setVelocity(QVector2D(m_player->velocity().x(), 0));
//             house->shatter(QPointF(m_player->position().x(), leftCorner.y()));

//             // 撞到天花板是硬碰撞，本帧不再检测其他表面，但后续的石头检测仍需执行
//             goto end_all_surface_checks;
//         }
//     }

//     // --- 阶段二：检测表面滑行 (屋顶, 翘板等) ---
//     // 优先检测房屋
//     for (House* house : m_houses) {
//         QPainterPath roofPath = house->getRoofPath();
//         auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
//         if (roofInfo.first.y() != 0 && fabs(m_player->position().y() - roofInfo.first.y()) < 5.0) {

//             house->shatter(m_player->position());

//             QPointF peakWorldPos = house->getRoofPeakWorldPosition();
//             if (m_player->position().x() < peakWorldPos.x()) {
//                 // 左侧滑行
//                 isPlayerOnASurface = true;
//                 QPointF playerPos = m_player->position();
//                 playerPos.setY(roofInfo.first.y());
//                 m_player->setPosition(playerPos);
//                 m_player->setRotation(roofInfo.second);

//                 QVector2D airVelocity = m_player->velocity();
//                 QVector2D groundDirection(qCos(qDegreesToRadians(roofInfo.second)), qSin(qDegreesToRadians(roofInfo.second)));
//                 qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//                 if (newSpeed < 0) newSpeed = 0;
//                 m_player->setVelocity(groundDirection * newSpeed);
//             } else {
//                 // 右侧挤压
//                 QPointF playerPos = m_player->position();
//                 playerPos.setY(roofInfo.first.y());
//                 m_player->setPosition(playerPos);
//             }
//             // 找到一个表面就跳出所有表面检测
//             goto end_all_surface_checks;
//         }
//     }

//     // 如果没在屋顶上，再检测翘板
//     for (Seesaw* seesaw : m_seesaws) {
//         if (seesaw->currentState() == Seesaw::Shattered) continue;

//         QPainterPath plankPath = seesaw->getPlankPath();
//         auto plankInfo = getPathInfoAt(plankPath, m_player->position().x());
//         if (plankInfo.first.y() != 0 && fabs(m_player->position().y() - plankInfo.first.y()) < 5.0) {

//             seesaw->shatter(m_player->position());
//             isPlayerOnASurface = true;

//             QPointF playerPos = m_player->position();
//             playerPos.setY(plankInfo.first.y());
//             m_player->setPosition(playerPos);
//             m_player->setRotation(plankInfo.second);

//             QVector2D airVelocity = m_player->velocity();
//             QVector2D groundDirection(qCos(qDegreesToRadians(plankInfo.second)), qSin(qDegreesToRadians(plankInfo.second)));
//             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//             if (newSpeed < 0) newSpeed = 0;
//             m_player->setVelocity(groundDirection * newSpeed);

//             goto end_all_surface_checks;
//         }
//     }

//     // --- 阶段三：如果不在任何障碍物表面上，检测雪地 ---
//     if (!isPlayerOnASurface) {
//         auto terrainInfo = getTerrainInfoAt(m_player->position().x());
//         qreal terrainY = terrainInfo.first.y();
//         if (m_player->position().y() >= terrainY - 1.0 && m_player->velocity().y() >= 0) {
//             isPlayerOnASurface = true;
//             // 雪地滑行逻辑 (不变)
//             QPointF playerPos = m_player->position();
//             playerPos.setY(terrainY);
//             m_player->setPosition(playerPos);
//             m_player->setRotation(terrainInfo.second);
//             QVector2D airVelocity = m_player->velocity();
//             QVector2D groundDirection(qCos(qDegreesToRadians(terrainInfo.second)), qSin(qDegreesToRadians(terrainInfo.second)));
//             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//             if (newSpeed < 0) newSpeed = 0;
//             m_player->setVelocity(groundDirection * newSpeed);
//         }
//     }

// end_all_surface_checks:
//     // --- 阶段四：最终更新状态并检测其他碰撞 ---
//     m_player->onGround = isPlayerOnASurface;

//     for (Obstacle* obs : m_obstacles) {
//         Stone* stone = dynamic_cast<Stone*>(obs);
//         if (stone && stone->currentState == Stone::Intact) {
//             if (m_player->collisionRect().intersects(stone->collisionRect())) {
//                 stone->shatter();
//             }
//         }
//     }
// }
// --- 新增：碰撞检测的实现 ---
// void GameScreen::checkCollisions()
// {
//     if (!m_player) return;
//     bool playerOnRoof = false;
//     // 默认玩家不在任何地面上，受重力影响
//     m_player->onGround = false;

//     // 1. 创建一个标志，用于记录本轮检测中玩家是否接触了任何可滑行表面
//     bool isPlayerOnASurface = false;

//     // --- 2. 优先检测与屋顶的碰撞 ---
//     for (House* house : m_houses) {
//         QPainterPath roofPath = house->getRoofPath(); // 获取屋顶的世界坐标路径
//         QPointF peakWorldPos = house->getRoofPeakWorldPosition(); // 获取屋顶最高点的世界坐标
//         auto roofInfo = getPathInfoAt(roofPath, m_player->position().x()); // 获取玩家当前X坐标对应的屋顶信息

//         // --- 首先处理从下方跳跃的碰撞 ---
//         QPointF leftCorner = house->getRoofLeftCornerWorldPosition();
//         QPointF rightCorner = house->getRoofRightCornerWorldPosition();

//         // 玩家的碰撞矩形顶部Y坐标
//         qreal playerTopY = m_player->collisionRect().top();
//         // 检查玩家是否正在向上移动，并且其X坐标在屋顶底边范围内
//         if (m_player->velocity().y() < 0 &&
//             m_player->position().x() > leftCorner.x() &&
//             m_player->position().x() < rightCorner.x()) {

//             // 如果玩家的头顶即将或已经接触到屋顶底边
//             if (playerTopY < leftCorner.y()) {
//                 // 碰撞生效
//                 QPointF playerPos = m_player->position();
//                 // 将玩家的位置校正到屋顶底边下方，防止穿透
//                 playerPos.setY(leftCorner.y() + m_player->collisionRect().height());
//                 m_player->setPosition(playerPos);
//                 // 阻止向上的速度，使其自然下落
//                 m_player->setVelocity(QVector2D(m_player->velocity().x(), 0));

//                 // 触发破碎效果
//                 house->shatter(QPointF(m_player->position().x(), leftCorner.y()));

//                 // 因为已经处理了天花板碰撞，直接跳到末尾
//                 goto end_surface_check;
//             }
//         }
//         // getPathInfoAt 在找不到有效路径段时会返回一个Y值为0的点, 我们用此判断玩家是否在屋顶的水平投影范围内
//         if (roofInfo.first.y() != 0) {
//             qreal roofY = roofInfo.first.y();

//             // // 【核心修正】判断玩家是否在屋顶上的新逻辑
//             // // 不再单纯依赖速度，而是检查玩家Y坐标是否与屋顶Y坐标足够接近 (允许5个像素的误差)
//             // if (fabs(m_player->position().y() - roofY) < 5.0) {

//             //     // --- 确认玩家在屋顶上，执行滑行逻辑 ---
//             //     isPlayerOnASurface = true; // 标记玩家在本帧接触到了表面

//             //     // 精确地将玩家放回屋顶表面
//             //     qreal roofAngle = roofInfo.second;
//             //     QPointF playerPos = m_player->position();
//             //     playerPos.setY(roofY);
//             //     m_player->setPosition(playerPos);
//             //     m_player->setRotation(roofAngle);

//             //     // 根据屋顶坡度重新计算滑行速度 (此部分逻辑是正确的，予以保留)
//             //     QVector2D airVelocity = m_player->velocity();
//             //     QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
//             //     qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//             //     if (newSpeed < 0) newSpeed = 0; // 防止上坡时倒滑
//             //     m_player->setVelocity(groundDirection * newSpeed);

//             //     // 既然已经在屋顶上，就无需再检测其他地面，直接跳到函数末尾的石头检测部分
//             //     goto end_surface_check;
//             // }

//             // 如果玩家当前位置与屋顶表面足够近，则视为接触
//             if (fabs(m_player->position().y() - roofY) < 5.0) {
//                 // --- 【核心新增】在这里调用房屋的破碎方法 ---
//                 // 将玩家的当前位置作为破碎效果的触发点
//                 house->shatter(m_player->position());
//                 // 【核心逻辑】判断玩家在屋顶的左侧还是右侧
//                 if (m_player->position().x() < peakWorldPos.x()) {
//                     // --- 逻辑A：在屋顶左侧，正常滑行 ---
//                     isPlayerOnASurface = true; // 标记玩家在地面上

//                     qreal roofAngle = roofInfo.second;
//                     QPointF playerPos = m_player->position();
//                     playerPos.setY(roofY); // 精确地放回表面
//                     m_player->setPosition(playerPos);
//                     m_player->setRotation(roofAngle);

//                     // 根据坡度重新计算速度
//                     QVector2D airVelocity = m_player->velocity();
//                     QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
//                     qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//                     if (newSpeed < 0) newSpeed = 0;
//                     m_player->setVelocity(groundDirection * newSpeed);

//                     // 已在屋顶上处理完毕，直接跳到末尾的石头检测
//                     goto end_surface_check;

//                 } else {
//                     // --- 逻辑B：在屋顶右侧，作为“墙”进行挤压 ---
//                     // 我们只校正玩家的位置以防止穿透，不改变他的速度、角度，也不认为他在“地面”上。
//                     // 这样他就会保持原有的运动轨迹，但被墙挡住。
//                     QPointF playerPos = m_player->position();
//                     playerPos.setY(roofY); // 仅将Y坐标校正到墙面，防止穿透
//                     m_player->setPosition(playerPos);
//                     // 【注意】这里不设置 isPlayerOnASurface = true，所以玩家依然受重力影响。
//                 }
//             }
//         }
//     }

//     for (Seesaw* seesaw : m_seesaws) {
//         if (seesaw->m_shatterState == Seesaw::Shattered) continue; // 跳过已破碎的

//         QPainterPath plankPath = seesaw->getPlankPath();
//         auto plankInfo = getPathInfoAt(plankPath, m_player->position().x());

//         if (plankInfo.first.y() != 0) {
//             qreal plankY = plankInfo.first.y();
//             if (fabs(m_player->position().y() - plankY) < 5.0) {
//                 // --- 逻辑：与翘板接触，执行滑行并触发破碎 ---

//                 // 触发破碎效果
//                 seesaw->shatter(m_player->position());

//                 // 执行滑行物理逻辑 (与屋顶左侧完全相同)
//                 isPlayerOnASurface = true;
//                 qreal plankAngle = plankInfo.second;
//                 QPointF playerPos = m_player->position();
//                 playerPos.setY(plankY);
//                 m_player->setPosition(playerPos);
//                 m_player->setRotation(plankAngle);

//                 QVector2D airVelocity = m_player->velocity();
//                 QVector2D groundDirection(qCos(qDegreesToRadians(plankAngle)), qSin(qDegreesToRadians(plankAngle)));
//                 qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//                 if (newSpeed < 0) newSpeed = 0;
//                 m_player->setVelocity(groundDirection * newSpeed);

//                 goto end_surface_check;
//             }
//         }
//     }

//     // --- 3. 如果不在任何屋顶上，再检测与雪地的碰撞 ---
//     // 这个 'if' 判断确保了只有在玩家没有接触任何屋顶时，才会进行雪地检测
//     if (!isPlayerOnASurface) {
//         auto terrainInfo = getTerrainInfoAt(m_player->position().x());
//         qreal terrainY = terrainInfo.first.y();

//         // 同样使用改进后的逻辑来判断
//         if (m_player->position().y() >= terrainY - 1.0 && m_player->velocity().y() >= 0) {
//             // --- 玩家在雪地上，执行滑行逻辑 ---
//             isPlayerOnASurface = true; // 标记玩家在本帧接触到了表面

//             // 与屋顶逻辑完全相同的物理处理
//             qreal terrainAngle = terrainInfo.second;
//             QPointF playerPos = m_player->position();
//             playerPos.setY(terrainY);
//             m_player->setPosition(playerPos);
//             m_player->setRotation(terrainAngle);

//             QVector2D airVelocity = m_player->velocity();
//             QVector2D groundDirection(qCos(qDegreesToRadians(terrainAngle)), qSin(qDegreesToRadians(terrainAngle)));
//             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//             if (newSpeed < 0) newSpeed = 0;
//             m_player->setVelocity(groundDirection * newSpeed);
//         }
//     }

// end_surface_check: // 标签，用于从屋顶检测直接跳转过来
//     // --- 4. 根据本轮检测结果，最终确定玩家的在/离地状态 ---
//     // 只有当 isPlayerOnASurface 为 true 时，才设置 onGround 为 true
//     m_player->onGround = isPlayerOnASurface;

//     // --- 5. 最后检测与石头的碰撞 (此逻辑与地面状态无关) ---
//     for (Obstacle* obs : m_obstacles) {
//         Stone* stone = dynamic_cast<Stone*>(obs);
//         if (stone && stone->currentState == Stone::Intact) {
//             if (m_player->collisionRect().intersects(stone->collisionRect())) {
//                 stone->shatter();
//             }
//         }
//     }

//     // // --- 1. 优先检测与屋顶的碰撞 ---
//     // for (House* house : m_houses) {
//     //     QPainterPath roofPath = house->getRoofPath();
//     //     auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());

//     //     // 检查玩家的X坐标是否在屋顶的水平范围内
//     //     if (roofInfo.first.y() != 0) { // getPathInfoAt 找到有效线段的标志
//     //         qreal roofY = roofInfo.first.y();
//     //         // 如果玩家是从上方接触屋顶
//     //         if (m_player->position().y() <= roofY + 0.05 && m_player->velocity().y() >= 0) {
//     //             // --- 玩家在屋顶上，执行滑行逻辑 ---
//     //             m_player->onGround = true; // 标记玩家在地面上

//     //             qreal roofAngle = roofInfo.second;
//     //             QPointF playerPos = m_player->position();
//     //             playerPos.setY(roofY);
//     //             m_player->setPosition(playerPos);
//     //             m_player->setRotation(roofAngle);

//     //             QVector2D airVelocity = m_player->velocity();
//     //             QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
//     //             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//     //             if (newSpeed < 0) newSpeed = 0;
//     //             m_player->setVelocity(groundDirection * newSpeed);

//     //             // 既然已经在屋顶上，就无需再检测其他地面，直接返回
//     //             return;
//     //         }
//     //     }
//     // }

//     // // --- 2. 如果不在屋顶上，再检测与雪地的碰撞 ---
//     // auto terrainInfo = getTerrainInfoAt(m_player->position().x());
//     // qreal terrainY = terrainInfo.first.y();

//     // if (m_player->position().y() >= terrainY && m_player->velocity().y() >= 0) {
//     //     // --- 玩家在雪地上，执行滑行逻辑 ---
//     //     m_player->onGround = true; // 标记玩家在地面上

//     //     qreal terrainAngle = terrainInfo.second;
//     //     QPointF playerPos = m_player->position();
//     //     playerPos.setY(terrainY);
//     //     m_player->setPosition(playerPos);
//     //     m_player->setRotation(terrainAngle);

//     //     QVector2D airVelocity = m_player->velocity();
//     //     QVector2D groundDirection(qCos(qDegreesToRadians(terrainAngle)), qSin(qDegreesToRadians(terrainAngle)));
//     //     qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//     //     if (newSpeed < 0) newSpeed = 0;
//     //     m_player->setVelocity(groundDirection * newSpeed);

//     //     return; // 在雪地上，也无需再检测
//     // }


//     // // --- 3. 最后检测与石头的碰撞 (此逻辑与地面状态无关) ---
//     // for (Obstacle* obs : m_obstacles) {
//     //     Stone* stone = dynamic_cast<Stone*>(obs);
//     //     if (stone && stone->currentState == Stone::Intact) {
//     //         if (m_player->collisionRect().intersects(stone->collisionRect())) {
//     //             stone->shatter();
//     //         }
//     //     }
//     // }


//     // for (Obstacle* obs : m_obstacles) {
//     //     // 将障碍物转换为石头类型以访问其特定属性
//     //     Stone* stone = dynamic_cast<Stone*>(obs);
//     //     if (stone && stone->currentState == Stone::Intact) {
//     //         // 只检测完好的石头
//     //         if (m_player->collisionRect().intersects(stone->collisionRect())) {
//     //             // 发生碰撞，触发破碎
//     //             stone->shatter();
//     //             // 在这里您可以添加更多碰撞后的逻辑，比如减速、播放音效等
//     //         }
//     //     }
//     // }

//     // // --- 碰撞检测：玩家 vs 房屋屋顶 ---
//     // for (House* house : m_houses) {
//     //     QPainterPath roofPath = house->getRoofPath(); // 获取世界坐标系中的屋顶路径

//     //     // 只有当玩家在下落，并且与屋顶相交时，才处理碰撞
//     //     if (m_player->velocity().y() >= 0 && roofPath.intersects(m_player->collisionRect()))
//     //     {
//     //         // 获取玩家脚底中心点在屋顶上的信息
//     //         auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
//     //         qreal roofY = roofInfo.first.y();

//     //         // 确保玩家确实是从上方接触屋顶
//     //         if (m_player->position().y() >= roofY - 5) { // 减去一个小的容差值
//     //             playerOnRoof = true; // 确认玩家在屋顶上

//     //             // --- 在这里，您可以根据条件判断是滑行还是破碎 ---
//     //             // if (玩家速度 > 某个值) {
//     //             //     house->shatter();
//     //             // } else {

//     //             // --- 执行滑行逻辑 (与雪地逻辑完全相同) ---
//     //             qreal roofAngle = roofInfo.second;
//     //             QPointF playerPos = m_player->position();

//     //             playerPos.setY(roofY);
//     //             m_player->setPosition(playerPos);
//     //             m_player->onGround = true; // 在屋顶上也算在“地面”
//     //             m_player->setRotation(roofAngle);

//     //             QVector2D airVelocity = m_player->velocity();
//     //             QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
//     //             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//     //             if (newSpeed < 0) newSpeed = 0;

//     //             m_player->setVelocity(groundDirection * newSpeed);

//     //             break; // 假设玩家一次只能在一个屋顶上，处理完就跳出循环
//     //             // }
//     //         }
//     //     }
//     // }

//     // // --- 碰撞检测：玩家 vs 雪地 ---
//     // // 只有当玩家没有在任何一个屋顶上时，才检测与雪地的碰撞
//     // if (!playerOnRoof) {
//     //     QPointF playerPos = m_player->position();
//     //     auto terrainInfo = getTerrainInfoAt(playerPos.x());
//     //     qreal terrainY = terrainInfo.first.y();

//     //     if (playerPos.y() >= terrainY && m_player->velocity().y() >= 0) {
//     //         // 雪地滑行逻辑 (保持不变)
//     //         m_player->onGround = true;
//     //         qreal terrainAngle = terrainInfo.second;
//     //         playerPos.setY(terrainY);
//     //         m_player->setPosition(playerPos);
//     //         m_player->setRotation(terrainAngle);
//     //         QVector2D airVelocity = m_player->velocity();
//     //         QVector2D groundDirection(qCos(qDegreesToRadians(terrainAngle)), qSin(qDegreesToRadians(terrainAngle)));
//     //         qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
//     //         if (newSpeed < 0) newSpeed = 0;
//     //         m_player->setVelocity(groundDirection * newSpeed);
//     //     } else {
//     //         m_player->onGround = false;
//     //     }
//     // }
// }

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

// --- 【新增】平缓雪地生成函数 (长度约 2160) ---
void GameScreen::generateGentleSlope(QList<QPointF>& points, const QPointF& startPoint)
{
    const qreal length = 1080 * 2;
    const qreal amplitude = 100; // 振幅，即雪地起伏的高度
    const int segments = 108;    // 分成108段，每段20像素

    for (int i = 0; i <= segments; ++i) {
        qreal x = startPoint.x() + i * (length / segments);
        // 使用余弦函数来确保起点和终点的切线是水平的
        // (i / segments) * M_PI 会使 x 从 0 变化到 PI，cos(x)的导数在0和PI处为0
        qreal y = startPoint.y() + amplitude * (1 - cos(i * (M_PI / segments)));
        points.append(QPointF(x, y));
    }
}

// --- 【新增】陡峭雪地生成函数 (长度 1080) ---
void GameScreen::generateSteepSlope(QList<QPointF>& points, const QPointF& startPoint)
{
    const qreal length = 1080;
    const qreal drop = 450; // 总下降高度
    const int segments = 54;

    for (int i = 0; i <= segments; ++i) {
        qreal progress = (qreal)i / segments; // 进度: 0.0 -> 1.0
        qreal x = startPoint.x() + progress * length;
        // 使用一个缓动曲线 (ease-in-out) 来构造斜坡，保证出入口平滑
        // 3x^2 - 2x^3 是一个常用的、出入口导数为0的平滑函数
        qreal y_offset = (3 * pow(progress, 2) - 2 * pow(progress, 3)) * drop;
        points.append(QPointF(x, startPoint.y() + y_offset));
    }
}

// --- 【新增】悬崖生成函数 (长度 200) ---
void GameScreen::generateCliff(QList<QPointF>& points, const QPointF& startPoint)
{
    const qreal length = 200;
    const qreal drop = 300; // 悬崖的下降高度
    const int segments = 10;

    for (int i = 0; i <= segments; ++i) {
        qreal progress = (qreal)i / segments;
        qreal x = startPoint.x() + progress * length;
        // 使用 progress 的平方来模拟开口向下的二次函数右半部分
        qreal y = startPoint.y() + pow(progress, 2) * drop;
        points.append(QPointF(x, y));
    }
}
