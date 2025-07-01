#include "game/Avalanche.h"
#include "ui/GameScreen.h"
#include <QDebug>
#include <QTimer>
Avalanche::Avalanche(GameScreen *gameScreen, QObject *parent)
    : GameObject(parent),
    m_gameScreen(gameScreen),
    m_baseAcceleration(0.002),
    m_currentAcceleration(m_baseAcceleration),
    m_verticalOffset(0.0)
{
    if (!m_originalPixmap.load(":/assets/images/avalanche.png")) {
        qWarning() << "Failed to load avalanche image! Creating a placeholder.";
        m_originalPixmap = QPixmap(1200, 200);
        m_originalPixmap.fill(Qt::white);
    }
    setScale(1.0);
}

qreal Avalanche::width() const
{
    return m_originalPixmap.width() * m_scale;
}

void Avalanche::update()
{
    if (!m_gameScreen) return;

    // 获取雪崩当前位置下方的地形信息 (Y坐标 和 坡度)
    auto terrainInfo = m_gameScreen->getTerrainInfoAt(m_position.x());
    qreal terrainY = terrainInfo.first.y();
    qreal terrainAngle = terrainInfo.second; // 坡度的角度

    m_position.setY(terrainY);
    setRotation(terrainAngle); // 让雪崩图片的朝向和坡度保持一致

    // 计算当前的速度大小，并施加加速度
    qreal currentSpeed = m_velocity.length();
    currentSpeed += m_currentAcceleration;

    // 根据当前地形的坡度，计算出速度的方向
    qreal angleRad = qDegreesToRadians(terrainAngle);
    QVector2D direction(qCos(angleRad), qSin(angleRad));

    // 结合新的速度大小和方向，得到最终的速度矢量
    setVelocity(direction * currentSpeed);

    // 根据最终的速度，更新雪崩的位置
    m_position += m_velocity.toPointF();
}


void Avalanche::applySlowDown(float slowDownFactor, int duration)
{
    qDebug() << "雪崩减速！效果持续" << duration << "毫秒。";
    // 在基础加速度上施加减速效果
    m_currentAcceleration += slowDownFactor;
    // 如果减速后加速度变为负数，则将其限制为0，防止雪崩倒退
    if (m_currentAcceleration < 0) {
        m_currentAcceleration = 0;
    }

    // 创建一个一次性定时器，在持续时间结束后恢复基础加速度
    QTimer::singleShot(duration, this, [this]() {
        m_currentAcceleration = m_baseAcceleration;
        qDebug() << "雪崩减速效果结束，恢复正常速度。";
    });
}

void Avalanche::pushBack(float distance)
{
    qDebug() << "冲击波！雪崩被推后了" << distance << "像素。";
    // 直接修改雪崩的X坐标，使其向左移动
    m_position.rx() -= distance;
}
