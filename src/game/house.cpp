#include "game/House.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include "audio/AudioManager.h"
// 破碎动画的常量
const int SHATTER_FADE_STEP_DURATION = 50; // 淡出效果每步之间的时间间隔 (毫秒)
const qreal SHATTER_OPACITY_STEP = 0.05;   // 每次降低多少透明度

House::House(QObject *parent)
    : Obstacle(parent), m_doorState(Closed),
    // 初始化破碎相关的成员变量
    m_shatterState(Intact),
    m_shatterOpacity(1.0)
{
    // 加载房屋主体图片
    if (!m_originalPixmap.load(":/assets/images/house.png")) {
        qWarning() << "Failed to load house image!";
    }

    // 加载门的图片
    if (!m_doorPixmap.load(":/assets/images/house_door.png")) {
        qWarning() << "Failed to load house door image!";
    }
    // 加载破碎效果的贴图
    if (!m_shatteredPixmap.load(":/assets/images/house_shattered.png")) {
        qWarning() << "Failed to load house shattered image!";
    }

    // 绘制屋顶的碰撞体积
    qreal width = m_originalPixmap.width();
    qreal height = m_originalPixmap.height();
    QPointF leftCorner(-width*0.4088, -height * 0.4218-height * 0.3218);
    QPointF topPeak(width*0.02, -height*0.6328-height * 0.3218);
    QPointF rightCorner(width*0.1276, -height * 0.4218-height * 0.3218);


    // 存储屋顶最高点的局部坐标
    m_roofPeak = topPeak;
    m_roofLeftCorner = leftCorner;
    m_roofRightCorner = rightCorner;
    m_roofPath.moveTo(leftCorner);
    m_roofPath.lineTo(topPeak);
    m_roofPath.lineTo(rightCorner);

    setScale(0.6);

    // 初始化并连接破碎计时器
    m_shatterTimer = new QTimer(this);
    connect(m_shatterTimer, &QTimer::timeout, this, &House::updateShatterEffect);

}


void House::shatter(const QPointF& point)
{
    // 只有完好的房子才能被破碎
    if (m_shatterState == Intact) {
        m_shatterState = Shattering;
        m_shatterPosition = point;
        m_shatterOpacity = 1.0;
        m_shatterTimer->start(SHATTER_FADE_STEP_DURATION); // 启动淡出动画
        AudioManager::instance()->playSoundEffect(SfxType::HouseShatter);
    }
}
// 实现获取屋顶左右底角世界坐标的函数
QPointF House::getRoofLeftCornerWorldPosition() const
{
    QTransform transform;
    transform.translate(m_position.x(), m_position.y());
    transform.scale(m_scale, m_scale);
    return transform.map(m_roofLeftCorner);
}

QPointF House::getRoofRightCornerWorldPosition() const
{
    QTransform transform;
    transform.translate(m_position.x(), m_position.y());
    transform.scale(m_scale, m_scale);
    return transform.map(m_roofRightCorner);
}
// 实现更新破碎效果的槽函数
void House::updateShatterEffect()
{
    m_shatterOpacity -= SHATTER_OPACITY_STEP; // 降低透明度

    if (m_shatterOpacity <= 0.0) {
        m_shatterOpacity = 0.0;
        m_shatterTimer->stop();         // 停止计时器
        m_shatterState = Shattered;     // 标记为“已破碎”
    }
}
// 实现获取屋顶最高点世界坐标的函数
QPointF House::getRoofPeakWorldPosition() const
{
    QTransform transform;
    transform.translate(m_position.x(), m_position.y());
    transform.scale(m_scale, m_scale);
    // 返回经过变换后的最高点坐标
    return transform.map(m_roofPeak);
}


QPainterPath House::getRoofPath() const
{
    // 创建一个变换矩阵
    QTransform transform;

    transform.translate(m_position.x(), m_position.y());
    // 再定义缩放：围绕新的原点（即物体中心）进行缩放
    transform.scale(m_scale, m_scale);

    // 返回经过“先缩放，后位移”变换后的路径
    return transform.map(m_roofPath);
}

void House::openDoor()
{
    if (m_doorState == Closed) {
        m_doorState = Open;
    }
}

void House::draw(QPainter* painter)
{
    // 先用基类的方法绘制房屋主体
    GameObject::draw(painter);


    const qreal doorOffsetX = -m_pixmap.width()*0.421875; // X方向偏移量：负数向左，正数向右
    const qreal doorOffsetY = -m_pixmap.height()*0.332; // Y方向偏移量：负数向上，正数向下

    // 如果门是打开状态，额外绘制门的贴图
    if (m_doorState == Open) {
        // 计算门的位置，这里假设门在房屋右侧底部
        qreal doorX = m_position.x() + m_pixmap.width() / 2.0+ doorOffsetX;
        qreal doorY = m_position.y() - m_doorPixmap.height()+ doorOffsetY;
        painter->drawPixmap(QPointF(doorX, doorY), m_doorPixmap);
    }


    // 如果房屋正在破碎，就在指定位置绘制带透明度的破碎贴图
    if (m_shatterState == Shattering) {
        painter->save(); // 保存当前的 painter 状态
        painter->setOpacity(m_shatterOpacity); // 设置透明度

        // 以破碎点为中心绘制破碎贴图
        QPointF drawPos = m_shatterPosition - QPointF(m_shatteredPixmap.width() / 2.0, m_shatteredPixmap.height() / 2.0);
        painter->drawPixmap(drawPos, m_shatteredPixmap);

        painter->restore(); // 恢复 painter 状态，避免影响其他物体
    }


    // Debug模式下绘制碰撞路径 ---
    painter->save();
    // 创建一支粗的、红色的画笔
    QPen debugPen(Qt::red);
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    painter->drawPath(getRoofPath());
    painter->restore();
}
