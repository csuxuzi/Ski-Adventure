#include "game/Seesaw.h"
#include <QPainter>
#include <QDebug>
#include <QTransform>
#include <QTimer>
#include "audio/AudioManager.h"
// 定义破碎动画的常量
const int SHATTER_FADE_STEP_DURATION_SEESAW = 50;
const qreal SHATTER_OPACITY_STEP_SEESAW = 0.05;

Seesaw::Seesaw(QObject *parent)
    : Obstacle(parent),
    m_shatterState(Intact),
    m_shatterOpacity(1.0)
{
    // 1. 加载翘板的主体图片
    // 注意：您需要准备一张名为 "seesaw.png" 的图片并放在资源文件中
    if (!m_originalPixmap.load(":/assets/images/seesaw.png")) {
        qWarning() << "Failed to load seesaw image! Creating a placeholder.";
        // 如果加载失败，创建一个临时的占位符图像
        m_originalPixmap = QPixmap(200, 20);
        m_originalPixmap.fill(Qt::darkYellow);
    }

    // 2. 加载破碎效果的贴图 (这里我们继续复用石头的破碎图)
    if (!m_shatteredPixmap.load(":/assets/images/seesaw_shattered.png")) {
        qWarning() << "Failed to load shattered image for seesaw!";
    }

    // 3. 【核心】根据图片尺寸，定义碰撞路径 (从左下到右上)
    qreal width = m_originalPixmap.width();
    qreal height = m_originalPixmap.height();

    // 我们将锚点(0,0)设在翘板的中心
    QPointF bottomLeft(-width / 2, 0);
    QPointF topRight(width*5 / 12, -height);

    m_plankPath.moveTo(bottomLeft);
    m_plankPath.lineTo(topRight);

    // 4. 初始化并连接破碎计时器
    m_shatterTimer = new QTimer(this);
    connect(m_shatterTimer, &QTimer::timeout, this, &Seesaw::updateShatterEffect);

    // 5. 设置初始缩放（如果需要的话）
    setScale(0.6); // 保持原始尺寸
}

QPainterPath Seesaw::getPlankPath() const
{
    QTransform transform;
    // 首先平移到物体在世界中的位置
    transform.translate(m_position.x(), m_position.y());
    // 然后应用物体的旋转（翘板可以旋转）
    transform.rotate(m_rotation);
    // 最后应用缩放
    transform.scale(m_scale, m_scale);
    return transform.map(m_plankPath);
}

void Seesaw::shatter(const QPointF& point)
{
    if (m_shatterState == Intact) {
        m_shatterState = Shattering;
        m_shatterPosition = point;
        m_shatterOpacity = 1.0;
        m_shatterTimer->start(SHATTER_FADE_STEP_DURATION_SEESAW);
        // 【新增】播放翘板破碎音效
        AudioManager::instance()->playSoundEffect(SfxType::SeesawShatter);
    }
}

void Seesaw::updateShatterEffect()
{
    m_shatterOpacity -= SHATTER_OPACITY_STEP_SEESAW;
    if (m_shatterOpacity <= 0.0) {
        m_shatterOpacity = 0.0;
        m_shatterTimer->stop();
        m_shatterState = Shattered;
    }
}

void Seesaw::draw(QPainter* painter)
{
    // 1. 如果翘板是完好的，就使用基类的方法绘制主体
    //if (m_shatterState == Intact) {
    GameObject::draw(painter);
    //}

    // 2. 如果正在破碎，就绘制破碎效果
    if (m_shatterState == Shattering) {
        // 仍然绘制主体，因为翘板只是部分破碎，不是完全消失
        GameObject::draw(painter);

        painter->save();
        painter->setOpacity(m_shatterOpacity);
        QPointF drawPos = m_shatterPosition - QPointF(m_shatteredPixmap.width() / 2.0, m_shatteredPixmap.height() / 2.0);
        painter->drawPixmap(drawPos, m_shatteredPixmap);
        painter->restore();
    }
    // 如果已经破碎 (Shattered)，则什么都不绘制，翘板消失
    // --- 【新增】绘制翘板的碰撞路径 (用于调试) ---
    painter->save();
    QPen debugPen(Qt::red);
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    painter->drawPath(getPlankPath());
    painter->restore();
}
