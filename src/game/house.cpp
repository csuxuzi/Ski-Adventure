#include "game/House.h"
#include <QPainter>
#include <QDebug>
#include <QTimer> // <-- 【新增】包含 QTimer 头文件
#include "audio/AudioManager.h"
// --- 【新增】定义破碎动画的常量 ---
const int SHATTER_FADE_STEP_DURATION = 50; // 淡出效果每步之间的时间间隔 (毫秒)
const qreal SHATTER_OPACITY_STEP = 0.05;   // 每次降低多少透明度

House::House(QObject *parent)
    : Obstacle(parent), m_doorState(Closed),
    // --- 【新增】初始化破碎相关的成员变量 ---
    m_shatterState(Intact),
    m_shatterOpacity(1.0)
{
    // 1. 加载房屋主体图片
    if (!m_originalPixmap.load(":/assets/images/house.png")) { // 请替换为您的房屋图片路径
        qWarning() << "Failed to load house image!";
    }

    // 2. 加载门的图片
    if (!m_doorPixmap.load(":/assets/images/house_door.png")) { // 请替换为您的门图片路径
        qWarning() << "Failed to load house door image!";
    }
    // --- 【新增】加载破碎效果的贴图 ---
    if (!m_shatteredPixmap.load(":/assets/images/house_shattered.png")) { // 复用石头的破碎图
        qWarning() << "Failed to load house shattered image!";
    }


    // 3. 【核心】定义屋顶的碰撞路径
    //    这里的坐标是相对于GameObject的锚点（底部中心）的。
    //    您需要根据您的房屋图片精确地调整下面几个点的坐标。
    //    假设屋顶是一个简单的三角形：
    // qreal width = m_originalPixmap.width();
    // qreal height = m_originalPixmap.height();
    // QPointF bottomCenter(0, 0);
    // QPointF leftCorner(-width*0.2888, -height * 0.4218);   // 屋顶左下角
    // QPointF topPeak(width*0.02, -height*0.6028);                     // 屋顶最高点
    // QPointF rightCorner(width*0.126, -height * 0.4218);  // 屋顶右下角

    // m_roofPath.moveTo(leftCorner);
    // m_roofPath.lineTo(topPeak);
    // m_roofPath.lineTo(rightCorner);
    // // 注意：我们不闭合路径，因为它只代表屋顶表面，而不是一个封闭区域。

    // // 4. 设置整个对象的显示尺寸（这不会影响碰撞路径）
    // setScale(0.6);

    // --- 【核心修正】修正执行顺序和逻辑 ---

    // 2. 先根据原始图片尺寸，定义屋顶路径的“形状”
    qreal width = m_originalPixmap.width();
    qreal height = m_originalPixmap.height();
    QPointF leftCorner(-width*0.4088, -height * 0.4218-height * 0.3218);
    QPointF topPeak(width*0.02, -height*0.6328-height * 0.3218);
    QPointF rightCorner(width*0.1276, -height * 0.4218-height * 0.3218);


    // --- 【修改】存储屋顶最高点的局部坐标 ---
    m_roofPeak = topPeak;
    m_roofLeftCorner = leftCorner;
    m_roofRightCorner = rightCorner;
    m_roofPath.moveTo(leftCorner);
    m_roofPath.lineTo(topPeak);
    m_roofPath.lineTo(rightCorner);

    // 3. 最后再调用setScale，它会同时缩放视觉图像和矩形碰撞体
    setScale(0.6);

    // --- 【新增】初始化并连接破碎计时器 ---
    m_shatterTimer = new QTimer(this);
    connect(m_shatterTimer, &QTimer::timeout, this, &House::updateShatterEffect);

}

// --- 【新增】实现 shatter 方法 ---
void House::shatter(const QPointF& point)
{
    // 只有完好的房子才能被破碎
    if (m_shatterState == Intact) {
        m_shatterState = Shattering;     // 状态切换为“正在破碎”
        m_shatterPosition = point;       // 记录碰撞点
        m_shatterOpacity = 1.0;          // 重置透明度为完全不透明
        m_shatterTimer->start(SHATTER_FADE_STEP_DURATION); // 启动淡出动画
        // 【新增】播放房屋破碎音效
        AudioManager::instance()->playSoundEffect(SfxType::HouseShatter);
    }
}
// --- 【新增】实现获取屋顶左右底角世界坐标的函数 ---
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
// --- 【新增】实现更新破碎效果的槽函数 ---
void House::updateShatterEffect()
{
    m_shatterOpacity -= SHATTER_OPACITY_STEP; // 降低透明度

    if (m_shatterOpacity <= 0.0) {
        m_shatterOpacity = 0.0;
        m_shatterTimer->stop();         // 停止计时器
        m_shatterState = Shattered;     // 标记为“已破碎”
    }
}
// --- 【新增】实现获取屋顶最高点世界坐标的函数 ---
QPointF House::getRoofPeakWorldPosition() const
{
    QTransform transform;
    // 【重要】使用与 getRoofPath() 完全相同的变换！
    transform.translate(m_position.x(), m_position.y());
    transform.scale(m_scale, m_scale);
    // 返回经过变换后的最高点坐标
    return transform.map(m_roofPeak);
}

// QPainterPath House::getRoofPath() const
// {
//     // 返回相对于世界坐标的屋顶路径
//     QTransform transform;
//     transform.translate(m_position.x(), m_position.y());
//     // 如果您的房子也可以旋转，这里也需要加上 transform.rotate(m_rotation);
//     return transform.map(m_roofPath);
// }

// --- 【核心修正】让getRoofPath返回缩放后的路径 ---
QPainterPath House::getRoofPath() const
{
    // 创建一个变换矩阵
    QTransform transform;

    // 1. 先应用缩放 (我们从基类获取存储的缩放值)
    // transform.scale(m_scale, m_scale);

    // // 2. 再应用位移，将其移动到世界坐标
    // transform.translate(m_position.x(), m_position.y());

    // --- 【核心修正】颠倒这里的顺序 ---
    // 1. 先定义平移：将坐标系移动到物体在世界中的位置
    //transform.translate(m_position.x(), m_position.y()-m_originalPixmap.height() * 0.4218/2);
    transform.translate(m_position.x(), m_position.y());
    // 2. 再定义缩放：围绕新的原点（即物体中心）进行缩放
    transform.scale(m_scale, m_scale);

    // 返回经过“先缩放，后位移”变换后的路径
    // 注意：这里的 m_roofPath 永远是基于原始尺寸的，我们只在“获取”它的时候才进行动态缩放
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
    // 1. 先用基类的方法绘制房屋主体
    GameObject::draw(painter);


    const qreal doorOffsetX = -m_pixmap.width()*0.421875; // X方向偏移量：负数向左，正数向右
    const qreal doorOffsetY = -m_pixmap.height()*0.332; // Y方向偏移量：负数向上，正数向下

    // 2. 如果门是打开状态，额外绘制门的贴图
    if (m_doorState == Open) {
        // 计算门的位置，这里假设门在房屋右侧底部
        qreal doorX = m_position.x() + m_pixmap.width() / 2.0+ doorOffsetX;
        qreal doorY = m_position.y() - m_doorPixmap.height()+ doorOffsetY;
        painter->drawPixmap(QPointF(doorX, doorY), m_doorPixmap);
    }


    // --- 【新增】绘制破碎效果 ---
    // 如果房屋正在破碎，就在指定位置绘制带透明度的破碎贴图
    if (m_shatterState == Shattering) {
        painter->save(); // 保存当前的 painter 状态
        painter->setOpacity(m_shatterOpacity); // 设置透明度

        // 以破碎点为中心绘制破碎贴图
        QPointF drawPos = m_shatterPosition - QPointF(m_shatteredPixmap.width() / 2.0, m_shatteredPixmap.height() / 2.0);
        painter->drawPixmap(drawPos, m_shatteredPixmap);

        painter->restore(); // 恢复 painter 状态，避免影响其他物体
    }


    // --- 【核心新增】Debug模式下绘制碰撞路径 ---
    painter->save();
    // 创建一支粗的、红色的画笔
    QPen debugPen(Qt::red);
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    // 将我们定义的屋顶路径绘制出来
    painter->drawPath(getRoofPath());
    painter->restore();
}
