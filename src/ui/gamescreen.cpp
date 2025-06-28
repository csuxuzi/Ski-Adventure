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
// 定义地形的一些常量，便于调整
const int TERRAIN_POINT_INTERVAL = 20; // 每个地形点的水平间距
const int TERRAIN_MIN_Y = 450;         // 地形的最低高度
const int TERRAIN_MAX_Y = 550;         // 地形的最高高度
const qreal SCROLL_SPEED = 2.0;        // 地面的滚动速度
// 定义玩家在屏幕上固定的X轴位置
const qreal PLAYER_FIXED_X = 200.0;

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent), m_backgroundOffset(0), m_worldOffset(0) // 初始化 m_worldOffset
{
    // 1. 加载背景图，如果失败则创建一个蓝色背景作为占位符
    if (!m_backgroundPixmap.load(":/assets/images/game_background.png")) {
        qWarning("Could not load game background image, creating a placeholder.");
        // 您期望的窗口尺寸是 1080x720，但主窗口目前是 800x600
        // 这里我们先使用主窗口的尺寸，便于查看
        m_backgroundPixmap = QPixmap(800, 600);
        m_backgroundPixmap.fill(QColor("#87CEEB")); // 天蓝色
    }

    // 2. 创建并连接计时器，用于驱动游戏循环
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameScreen::updateGame);
    connect(m_timer, &QTimer::timeout, this, &GameScreen::updateSnow);

    // 3. 初始化地形
    generateInitialTerrain();

    // 4. 创建并放置玩家
    placePlayer();

    setFocusPolicy(Qt::StrongFocus); // 确保 GameScreen 能接收键盘事件

    // --- 新增：在构造时调用，放置所有障碍物 ---
    setupObstacles();
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
        QPointF playerPos = m_player->position();
        auto terrainInfo = getTerrainInfoAt(playerPos.x());
        qreal terrainY = terrainInfo.first.y();
        qreal terrainAngle = terrainInfo.second;

        // // 3. 让玩家“粘”在地面上
        // //    这里是一个简化的实现，直接将玩家的Y坐标设置为地形的高度。
        // //    后续可以加入重力，当玩家在空中时，此逻辑不生效。
        // playerPos.setY(terrainY);
        // m_player->setPosition(playerPos);

        // // 4. 根据脚下的坡度，实时更新玩家的旋转角度和速度方向
        // m_player->setRotation(terrainAngle);

        // // 获取当前速度大小
        // qreal currentSpeed = m_player->velocity().length();
        // // 将速度大小和新的坡度角度结合，计算出新的速度矢量
        // qreal angleRad = qDegreesToRadians(terrainAngle);
        // QVector2D newVelocity(currentSpeed * qCos(angleRad), currentSpeed * qSin(angleRad));
        // m_player->setVelocity(newVelocity);

        // 3. 【核心】地面检测与处理
        if (playerPos.y() >= terrainY && m_player->velocity().y() >= 0) {
            // 条件：玩家位置在地面下方 或 与地面平齐，并且玩家正在下落（或水平移动）
            // 意味着玩家“着陆”了

            // 将玩家精确地放回地面
            playerPos.setY(terrainY);
            m_player->setPosition(playerPos);

            // 标记玩家在地面上
            m_player->onGround = true;

            // 根据脚下的坡度，调整玩家的旋转和速度
            m_player->setRotation(terrainAngle);

            // // 获取当前速度大小，并根据新坡度重新计算速度矢量
            // // 这使得玩家着陆后能顺着斜坡滑行
            // qreal currentSpeed = m_player->velocity().length();
            // qreal angleRad = qDegreesToRadians(terrainAngle);
            // QVector2D newVelocity(currentSpeed * qCos(angleRad), currentSpeed * qSin(angleRad));

            // // 当玩家在地面上时，我们不再关心垂直速度，所以可以只保留水平速度，或者直接使用新计算的速度
            // m_player->setVelocity(newVelocity);
            // 1. 获取着陆前的空中速度矢量
            QVector2D airVelocity = m_player->velocity();

            // 2. 计算出地面坡度的单位方向向量
            qreal angleRad = qDegreesToRadians(terrainAngle);
            QVector2D groundDirection(qCos(angleRad), qSin(angleRad));

            // 3. 计算空中速度在地面方向上的投影（点积）
            qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);

            // 安全检查：如果玩家几乎是逆着山坡撞上去，速度可能为负，我们不希望他倒滑，所以取0
            if (newSpeed < 0) {
                newSpeed = 0;
            }

            // 4. 用这个新的速度大小和地面方向来生成最终的速度矢量
            QVector2D newVelocity = groundDirection * newSpeed;
            m_player->setVelocity(newVelocity);


        } else {
            // 如果不在上述条件内，说明玩家在空中
            m_player->onGround = false;
        }

        // --- 3. 【核心修改】更新世界偏移量 ---
        // 世界的偏移量 = 玩家在世界中的X坐标 - 玩家在屏幕上固定的X坐标
        m_worldOffset = m_player->position().x() - PLAYER_FIXED_X;

        // 【核心修正】让背景图以一个恒定的慢速独立滚动
        // m_backgroundOffset += 1.5; // 您可以调整这个值来控制背景滚动的快慢
        // if (m_backgroundOffset >= width()) { // 相应地，这里的判断条件也要调整
        //     m_backgroundOffset -= width();
        // }
        checkCollisions();
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
int i = 0;
void GameScreen::updateSnow()
{
    // --- 【核心修改】现在雪地的生成和销毁要基于摄像机的位置 ---
    // 移除已经完全移出摄像机左侧的点
    while (m_snowPoints.size() > 2 && m_snowPoints[1].x() < m_worldOffset) {
        m_snowPoints.removeFirst();
    }

    // 在摄像机右侧补充新的点

    while (m_snowPoints.last().x() < m_worldOffset + width() + TERRAIN_POINT_INTERVAL) {
        qreal lastX = m_snowPoints.last().x();
        qreal lastY = m_snowPoints.last().y();
        // 此处可以加入更丰富的地形生成算法
        // qreal newY = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
        // newY = (lastY * 3 + newY) / 4; // 做一个简单的平滑处理
        if(i>100){
            i = 0;
        }
        qreal newY = lastY;
        if(i<=50){
            newY += i/10;
        }else{
            //newY -= (i-50)/10+0.3;
        }
        i++;
        m_snowPoints.append(QPointF(lastX + TERRAIN_POINT_INTERVAL, newY));

    }

    // 根据更新后的点，重新生成可绘制的路径
    updateSnowPath();


    // // 1. 将所有地形点的X坐标向左移动，实现滚动
    // for (int i = 0; i < m_snowPoints.size(); ++i) {
    //     m_snowPoints[i].setX(m_snowPoints[i].x() - SCROLL_SPEED);
    // }

    // // 2. 移除已经完全移出屏幕左侧的点
    // // 我们多保留一个点，确保曲线在屏幕外依然平滑
    // if (!m_snowPoints.isEmpty() && m_snowPoints.first().x() < -TERRAIN_POINT_INTERVAL) {
    //     m_snowPoints.removeFirst();
    // }

    // // 3. 在右侧补充新的点，以创建无限地形
    // while (!m_snowPoints.isEmpty() && m_snowPoints.last().x() < width()) {
    //     qreal lastX = m_snowPoints.last().x();
    //     qreal lastY = m_snowPoints.last().y();

    //     // 生成一个稍微有点起伏的新Y坐标
    //     //qreal newY = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
    //     // 为了平滑，可以取上一个点和新随机点的平均值
    //     //newY = (lastY + newY) / 2;

    //     //测试平地
    //     qreal newY = lastY;

    //     m_snowPoints.append(QPointF(lastX + TERRAIN_POINT_INTERVAL, newY));
    // }

    // // 4. 根据更新后的点，重新生成可绘制的路径
    // updateSnowPath();
}

void GameScreen::generateInitialTerrain()
{
    // 清空旧数据
    m_snowPoints.clear();

    // 生成从屏幕左侧到右侧，并额外多一个点的地形数据
    for (int x = 0; x <= width() + TERRAIN_POINT_INTERVAL; x += TERRAIN_POINT_INTERVAL) {
        //int y = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
        //测试平地
        int y = 500;
        m_snowPoints.append(QPointF(x, y));
    }

    // 根据初始点生成路径
    updateSnowPath();
}

void GameScreen::updateSnowPath()
{
    if (m_snowPoints.size() < 2) return;

    // 开始构建路径
    m_snowPath = QPainterPath();
    // 1. 将起点移动到屏幕左下角
    m_snowPath.moveTo(m_snowPoints.first().x(), height());
    // 2. 画一条线到第一个地形点
    m_snowPath.lineTo(m_snowPoints.first());
    // 3. 将所有地形点连接成一条曲线
    //    为了让曲线更平滑，这里可以使用 cubicTo，但先用 lineTo 实现基础功能
    for (int i = 1; i < m_snowPoints.size(); ++i) {
        m_snowPath.lineTo(m_snowPoints[i]);
    }
    // 4. 从最后一个地形点画一条线到屏幕右下角
    m_snowPath.lineTo(m_snowPoints.last().x(), height());
    // 5. 闭合路径，形成一个封闭的多边形
    m_snowPath.closeSubpath();
}

void GameScreen::placePlayer()
{
    m_player = new Player(this);

    // 将玩家放置在屏幕大约 1/4 的位置
    qreal player_x = width() / 4.0;

    // 获取该位置的地形信息（Y坐标和坡度）
    auto terrain_info = getTerrainInfoAt(player_x);

    // 设置玩家的初始位置和旋转
    m_player->setPosition(terrain_info.first);
    m_player->setRotation(terrain_info.second);

    // --- 设置初始速度 (代码简化和优化) ---
    qreal initialSpeed = 4.5; // 初始速度大小
    qreal initialAngleRad = qDegreesToRadians(m_player->rotation()); // 将初始角度转为弧度

    // 使用简单的2D三角函数计算速度矢量的x和y分量
    QVector2D initial_velocity(initialSpeed * qCos(initialAngleRad),
                               initialSpeed * qSin(initialAngleRad));

    m_player->setVelocity(initial_velocity);
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
    painter.translate(-m_worldOffset, 0);

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
    // 3. 在这里绘制未来的企鹅、雪怪等...
    //   (它们也会被自动正确地放置在滚动的世界中)

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
    house1->setPosition(QPointF(2000, getTerrainInfoAt(600).first.y()));
    m_houses.append(house1);

    // 您可以根据需要添加更多的石头或其他障碍物
}

// --- 新增：碰撞检测的实现 ---
void GameScreen::checkCollisions()
{
    if (!m_player) return;
    bool playerOnRoof = false;
    // 默认玩家不在任何地面上，受重力影响
    m_player->onGround = false;

    // --- 1. 优先检测与屋顶的碰撞 ---
    for (House* house : m_houses) {
        QPainterPath roofPath = house->getRoofPath();
        auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());

        // 检查玩家的X坐标是否在屋顶的水平范围内
        if (roofInfo.first.y() != 0) { // getPathInfoAt 找到有效线段的标志
            qreal roofY = roofInfo.first.y();
            // 如果玩家是从上方接触屋顶
            if (m_player->position().y() <= roofY + 0.05 && m_player->velocity().y() >= 0) {
                // --- 玩家在屋顶上，执行滑行逻辑 ---
                m_player->onGround = true; // 标记玩家在地面上

                qreal roofAngle = roofInfo.second;
                QPointF playerPos = m_player->position();
                playerPos.setY(roofY);
                m_player->setPosition(playerPos);
                m_player->setRotation(roofAngle);

                QVector2D airVelocity = m_player->velocity();
                QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
                qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
                if (newSpeed < 0) newSpeed = 0;
                m_player->setVelocity(groundDirection * newSpeed);

                // 既然已经在屋顶上，就无需再检测其他地面，直接返回
                return;
            }
        }
    }

    // --- 2. 如果不在屋顶上，再检测与雪地的碰撞 ---
    auto terrainInfo = getTerrainInfoAt(m_player->position().x());
    qreal terrainY = terrainInfo.first.y();

    if (m_player->position().y() >= terrainY && m_player->velocity().y() >= 0) {
        // --- 玩家在雪地上，执行滑行逻辑 ---
        m_player->onGround = true; // 标记玩家在地面上

        qreal terrainAngle = terrainInfo.second;
        QPointF playerPos = m_player->position();
        playerPos.setY(terrainY);
        m_player->setPosition(playerPos);
        m_player->setRotation(terrainAngle);

        QVector2D airVelocity = m_player->velocity();
        QVector2D groundDirection(qCos(qDegreesToRadians(terrainAngle)), qSin(qDegreesToRadians(terrainAngle)));
        qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
        if (newSpeed < 0) newSpeed = 0;
        m_player->setVelocity(groundDirection * newSpeed);

        return; // 在雪地上，也无需再检测
    }


    // --- 3. 最后检测与石头的碰撞 (此逻辑与地面状态无关) ---
    for (Obstacle* obs : m_obstacles) {
        Stone* stone = dynamic_cast<Stone*>(obs);
        if (stone && stone->currentState == Stone::Intact) {
            if (m_player->collisionRect().intersects(stone->collisionRect())) {
                stone->shatter();
            }
        }
    }


    // for (Obstacle* obs : m_obstacles) {
    //     // 将障碍物转换为石头类型以访问其特定属性
    //     Stone* stone = dynamic_cast<Stone*>(obs);
    //     if (stone && stone->currentState == Stone::Intact) {
    //         // 只检测完好的石头
    //         if (m_player->collisionRect().intersects(stone->collisionRect())) {
    //             // 发生碰撞，触发破碎
    //             stone->shatter();
    //             // 在这里您可以添加更多碰撞后的逻辑，比如减速、播放音效等
    //         }
    //     }
    // }

    // // --- 碰撞检测：玩家 vs 房屋屋顶 ---
    // for (House* house : m_houses) {
    //     QPainterPath roofPath = house->getRoofPath(); // 获取世界坐标系中的屋顶路径

    //     // 只有当玩家在下落，并且与屋顶相交时，才处理碰撞
    //     if (m_player->velocity().y() >= 0 && roofPath.intersects(m_player->collisionRect()))
    //     {
    //         // 获取玩家脚底中心点在屋顶上的信息
    //         auto roofInfo = getPathInfoAt(roofPath, m_player->position().x());
    //         qreal roofY = roofInfo.first.y();

    //         // 确保玩家确实是从上方接触屋顶
    //         if (m_player->position().y() >= roofY - 5) { // 减去一个小的容差值
    //             playerOnRoof = true; // 确认玩家在屋顶上

    //             // --- 在这里，您可以根据条件判断是滑行还是破碎 ---
    //             // if (玩家速度 > 某个值) {
    //             //     house->shatter();
    //             // } else {

    //             // --- 执行滑行逻辑 (与雪地逻辑完全相同) ---
    //             qreal roofAngle = roofInfo.second;
    //             QPointF playerPos = m_player->position();

    //             playerPos.setY(roofY);
    //             m_player->setPosition(playerPos);
    //             m_player->onGround = true; // 在屋顶上也算在“地面”
    //             m_player->setRotation(roofAngle);

    //             QVector2D airVelocity = m_player->velocity();
    //             QVector2D groundDirection(qCos(qDegreesToRadians(roofAngle)), qSin(qDegreesToRadians(roofAngle)));
    //             qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
    //             if (newSpeed < 0) newSpeed = 0;

    //             m_player->setVelocity(groundDirection * newSpeed);

    //             break; // 假设玩家一次只能在一个屋顶上，处理完就跳出循环
    //             // }
    //         }
    //     }
    // }

    // // --- 碰撞检测：玩家 vs 雪地 ---
    // // 只有当玩家没有在任何一个屋顶上时，才检测与雪地的碰撞
    // if (!playerOnRoof) {
    //     QPointF playerPos = m_player->position();
    //     auto terrainInfo = getTerrainInfoAt(playerPos.x());
    //     qreal terrainY = terrainInfo.first.y();

    //     if (playerPos.y() >= terrainY && m_player->velocity().y() >= 0) {
    //         // 雪地滑行逻辑 (保持不变)
    //         m_player->onGround = true;
    //         qreal terrainAngle = terrainInfo.second;
    //         playerPos.setY(terrainY);
    //         m_player->setPosition(playerPos);
    //         m_player->setRotation(terrainAngle);
    //         QVector2D airVelocity = m_player->velocity();
    //         QVector2D groundDirection(qCos(qDegreesToRadians(terrainAngle)), qSin(qDegreesToRadians(terrainAngle)));
    //         qreal newSpeed = QVector2D::dotProduct(airVelocity, groundDirection);
    //         if (newSpeed < 0) newSpeed = 0;
    //         m_player->setVelocity(groundDirection * newSpeed);
    //     } else {
    //         m_player->onGround = false;
    //     }
    // }
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
