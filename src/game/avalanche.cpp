#include "game/Avalanche.h"
#include "ui/GameScreen.h" // 需要包含 GameScreen 头文件以调用其方法
#include <QDebug>

Avalanche::Avalanche(GameScreen *gameScreen, QObject *parent)
    : GameObject(parent),
    m_gameScreen(gameScreen),
    m_acceleration(0.002), // 一个较小的加速度，可以按需调整
    m_verticalOffset(0.0)  // 雪崩比地形高80个像素
{
    // 1. 创建雪崩的视觉图像
    // 我们用代码创建一个半透明的、颜色不同的弧线作为占位符
    // 您可以替换为自己的 QPixmap 图片
    m_originalPixmap = QPixmap(1500, 300); // 雪崩的尺寸
    m_originalPixmap.fill(Qt::transparent); // 全透明背景

    QPainter painter(&m_originalPixmap);
    // 设置一个比雪地颜色稍暗的颜色，并设为半透明
    QColor avalancheColor(220, 220, 255, 180);
    painter.setBrush(avalancheColor);
    painter.setPen(Qt::NoPen);

    // 画一个简单的弧形作为雪崩的前缘
    QPainterPath path;
    path.moveTo(0, 250); // 左侧端点
    // 使用三次贝塞尔曲线来创建非对称的、更真实的雪崩弧线
    // 第一个控制点(500, -150)决定了左侧的平缓趋势
    // 第二个控制点(1200, 50)决定了右侧的陡峭趋势
    path.cubicTo(500, -150, 1350, 50, 1500, 250);
    painter.drawPath(path);

    // 2. 初始化缩放和位置
    setScale(1.0);
}

qreal Avalanche::width() const
{
    return m_originalPixmap.width() * m_scale;
}

void Avalanche::update()
{
    // 1. 根据加速度，增加X轴的速度
    QVector2D currentVelocity = velocity();
    currentVelocity.setX(currentVelocity.x() + m_acceleration);
    setVelocity(currentVelocity);

    // 2. 根据速度，更新位置
    m_position += m_velocity.toPointF();

    // 3. 更新Y轴位置，使其始终跟随在地形上方
    if (m_gameScreen) {
        // 获取雪崩中心点下方地形的高度
        qreal terrainY = m_gameScreen->getTerrainInfoAt(m_position.x()).first.y();
        // 设置自己的Y坐标为地形高度加上一个偏移量
        m_position.setY(terrainY + m_verticalOffset);
    }
}
