#include "ui/GameScreen.h"
#include "game/Player.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <QtMath> // 用于计算角度
#include <QVector3D> // <-- 在这里添加这一行
// 定义地形的一些常量，便于调整
const int TERRAIN_POINT_INTERVAL = 20; // 每个地形点的水平间距
const int TERRAIN_MIN_Y = 450;         // 地形的最低高度
const int TERRAIN_MAX_Y = 550;         // 地形的最高高度
const qreal SCROLL_SPEED = 2.0;        // 地面的滚动速度

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent), m_backgroundOffset(0)
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
    // 背景滚动逻辑
    m_backgroundOffset -= 0.5; // 让背景滚动得比地面慢，产生视差效果
    if (m_backgroundOffset <= -width()) {
        m_backgroundOffset = 0;
    }

    // 更新玩家状态
    if(m_player) {
        m_player->update();
    }


    // 请求重新绘制界面
    update();
}


void GameScreen::updateSnow()
{
    // 1. 将所有地形点的X坐标向左移动，实现滚动
    for (int i = 0; i < m_snowPoints.size(); ++i) {
        m_snowPoints[i].setX(m_snowPoints[i].x() - SCROLL_SPEED);
    }

    // 2. 移除已经完全移出屏幕左侧的点
    // 我们多保留一个点，确保曲线在屏幕外依然平滑
    if (!m_snowPoints.isEmpty() && m_snowPoints.first().x() < -TERRAIN_POINT_INTERVAL) {
        m_snowPoints.removeFirst();
    }

    // 3. 在右侧补充新的点，以创建无限地形
    while (!m_snowPoints.isEmpty() && m_snowPoints.last().x() < width()) {
        qreal lastX = m_snowPoints.last().x();
        qreal lastY = m_snowPoints.last().y();

        // 生成一个稍微有点起伏的新Y坐标
        //qreal newY = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
        // 为了平滑，可以取上一个点和新随机点的平均值
        //newY = (lastY + newY) / 2;
        qreal newY = lastY;

        m_snowPoints.append(QPointF(lastX + TERRAIN_POINT_INTERVAL, newY));
    }

    // 4. 根据更新后的点，重新生成可绘制的路径
    updateSnowPath();
}

void GameScreen::generateInitialTerrain()
{
    // 清空旧数据
    m_snowPoints.clear();

    // 生成从屏幕左侧到右侧，并额外多一个点的地形数据
    for (int x = 0; x <= width() + TERRAIN_POINT_INTERVAL; x += TERRAIN_POINT_INTERVAL) {
        int y = QRandomGenerator::global()->bounded(TERRAIN_MIN_Y, TERRAIN_MAX_Y);
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

    m_player->setPosition(terrain_info.first);
    m_player->setRotation(terrain_info.second);

    // 初始速度可以设置为沿着坡度方向的一个值
    QVector2D initial_velocity(5.0, 0); // 假设初始速度大小为5
    // 将速度矢量旋转到与地面平行
    QMatrix4x4 rotation_matrix;
    rotation_matrix.rotate(m_player->rotation(), 0, 0, 1);
    // 【最终修正代码】
    // 1. 将 2D 速度向量提升为 3D
    QVector3D velocity_3d(initial_velocity);
    // 2. 使用 map() 对 3D 向量进行旋转
    QVector3D rotated_velocity_3d = rotation_matrix.map(velocity_3d);
    // 3. 将旋转后的 3D 向量转换回 2D 并设置给玩家
    m_player->setVelocity(rotated_velocity_3d.toVector2D());
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
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，让曲线更平滑

    // 1. 绘制滚动的背景
    qreal x1 = m_backgroundOffset;
    qreal x2 = m_backgroundOffset + width();
    painter.drawPixmap(QRectF(x1, 0, width(), height()), m_backgroundPixmap, m_backgroundPixmap.rect());
    painter.drawPixmap(QRectF(x2, 0, width(), height()), m_backgroundPixmap, m_backgroundPixmap.rect());

    // 2. 绘制程序化生成的雪地
    painter.setBrush(Qt::white); // 设置填充颜色为白色
    painter.setPen(Qt::NoPen);   // 我们不需要边框线
    painter.drawPath(m_snowPath);

    // 3. 绘制玩家
    if(m_player) {
        m_player->draw(&painter);
    }
}
