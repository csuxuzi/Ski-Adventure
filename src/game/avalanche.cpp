#include "game/Avalanche.h"
#include "ui/GameScreen.h" // 需要包含 GameScreen 头文件以调用其方法
#include <QDebug>

Avalanche::Avalanche(GameScreen *gameScreen, QObject *parent)
    : GameObject(parent),
    m_gameScreen(gameScreen),
    m_acceleration(0.002), // 一个较小的加速度，可以按需调整
    m_verticalOffset(0.0)  // 雪崩比地形高80个像素
{
    if (!m_originalPixmap.load(":/assets/images/avalanche.png")) {
        qWarning() << "Failed to load avalanche image! Creating a placeholder.";
        // 如果加载失败，创建一个简单的白色方块作为占位符
        m_originalPixmap = QPixmap(1200, 200); // 雪崩的尺寸
        m_originalPixmap.fill(Qt::white);
    }

    // 2. 初始化缩放
    //    GameObject的锚点在底部中心，这对于沿地面滑行的雪崩来说非常完美
    setScale(1.0);


    // // 1. 创建雪崩的视觉图像
    // // 我们用代码创建一个半透明的、颜色不同的弧线作为占位符
    // // 您可以替换为自己的 QPixmap 图片
    // m_originalPixmap = QPixmap(1500, 300); // 雪崩的尺寸
    // m_originalPixmap.fill(Qt::transparent); // 全透明背景

    // QPainter painter(&m_originalPixmap);
    // // 设置一个比雪地颜色稍暗的颜色，并设为半透明
    // QColor avalancheColor(220, 220, 255, 180);
    // painter.setBrush(avalancheColor);
    // painter.setPen(Qt::NoPen);

    // // 画一个简单的弧形作为雪崩的前缘
    // QPainterPath path;
    // path.moveTo(0, 250); // 左侧端点
    // // 使用三次贝塞尔曲线来创建非对称的、更真实的雪崩弧线
    // // 第一个控制点(500, -150)决定了左侧的平缓趋势
    // // 第二个控制点(1200, 50)决定了右侧的陡峭趋势
    // path.cubicTo(500, -150, 1350, 50, 1500, 250);
    // painter.drawPath(path);

    // // 2. 初始化缩放和位置
    // setScale(1.0);
}

qreal Avalanche::width() const
{
    return m_originalPixmap.width() * m_scale;
}

// 【修改后】的update函数，实现贴地加速滑行
void Avalanche::update()
{
    if (!m_gameScreen) return;

    // 1. 获取雪崩当前位置下方的地形信息 (Y坐标 和 坡度)
    auto terrainInfo = m_gameScreen->getTerrainInfoAt(m_position.x());
    qreal terrainY = terrainInfo.first.y();
    qreal terrainAngle = terrainInfo.second; // 坡度的角度

    // 2. 让雪崩“吸附”在地面上
    m_position.setY(terrainY);
    setRotation(terrainAngle); // 让雪崩图片的朝向和坡度保持一致

    // 3. 计算当前的速度大小，并施加加速度
    //    我们只在速度大小上应用加速度，方向由地形决定
    qreal currentSpeed = m_velocity.length();
    currentSpeed += m_acceleration;

    // 4. 根据当前地形的坡度，计算出速度的方向
    qreal angleRad = qDegreesToRadians(terrainAngle);
    QVector2D direction(qCos(angleRad), qSin(angleRad));

    // 5. 结合新的速度大小和方向，得到最终的速度矢量
    setVelocity(direction * currentSpeed);

    // 6. 根据最终的速度，更新雪崩的位置
    m_position += m_velocity.toPointF();
}
