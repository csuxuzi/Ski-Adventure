#include "game/Signboard.h"
#include <QPainter>
#include <QDebug>
#include <QTransform>
#include <QTimer>
#include "audio/AudioManager.h"
const int SHATTER_FADE_STEP_DURATION_SIGN = 50;
const qreal SHATTER_OPACITY_STEP_SIGN = 0.05;

Signboard::Signboard(QObject *parent)
    : Obstacle(parent),
    m_shatterState(Intact),
    m_shatterOpacity(1.0)
{
    // 1. 加载告示牌的主体图片
    // 【注意】您需要准备一张名为 "signboard.png" 的图片并放到资源文件中
    if (!m_originalPixmap.load(":/assets/images/signboard.png")) {
        qWarning() << "Failed to load signboard image! Creating a placeholder.";
        m_originalPixmap = QPixmap(150, 20); // 创建一个占位符
        m_originalPixmap.fill(Qt::darkCyan);
    }

    // 2. 加载破碎效果的贴图
    if (!m_shatteredPixmap.load(":/assets/images/signboard_shattered.png")) { // 这里可以复用翘板的破碎图
        qWarning() << "Failed to load shattered image for signboard!";
    }

    // 3. 定义碰撞路径 (一条从左到右的直线)
    qreal width = m_originalPixmap.width();
    qreal height = m_originalPixmap.height();
    m_boardPath.moveTo(-width / 2, -height*0.95);
    m_boardPath.lineTo(width / 2, -height*0.95);

    // 4. 初始化破碎计时器
    m_shatterTimer = new QTimer(this);
    connect(m_shatterTimer, &QTimer::timeout, this, &Signboard::updateShatterEffect);

    // 5. 设置初始缩放
    setScale(1.2);
}

QPainterPath Signboard::getBoardPath() const
{
    QTransform transform;
    transform.translate(m_position.x(), m_position.y());
    transform.rotate(m_rotation);
    transform.scale(m_scale, m_scale);
    return transform.map(m_boardPath);
}

void Signboard::shatter(const QPointF& point)
{
    if (m_shatterState == Intact) {
        m_shatterState = Shattering;
        m_shatterPosition = point;
        m_shatterOpacity = 1.0;
        m_shatterTimer->start(SHATTER_FADE_STEP_DURATION_SIGN);
        // 【新增】播放破碎音效 (复用翘板的)
        AudioManager::instance()->playSoundEffect(SfxType::SeesawShatter);
    }
}

void Signboard::updateShatterEffect()
{
    m_shatterOpacity -= SHATTER_OPACITY_STEP_SIGN;
    if (m_shatterOpacity <= 0.0) {
        m_shatterOpacity = 0.0;
        m_shatterTimer->stop();
        m_shatterState = Shattered;
    }
}

void Signboard::draw(QPainter* painter)
{
    if (m_shatterState == Intact) {
        GameObject::draw(painter);
    }

    if (m_shatterState == Shattering) {
        GameObject::draw(painter);
        painter->save();
        painter->setOpacity(m_shatterOpacity);
        QPointF drawPos = m_shatterPosition - QPointF(m_shatteredPixmap.width() / 2.0, m_shatteredPixmap.height() / 2.0);
        painter->drawPixmap(drawPos, m_shatteredPixmap);
        painter->restore();
    }

    // (Debug) 绘制碰撞路径
    painter->save();
    QPen debugPen(Qt::blue); // 用蓝色以作区分
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    painter->drawPath(getBoardPath());
    painter->restore();
}
