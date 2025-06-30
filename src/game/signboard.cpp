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
    setScale(1.5);
}


void Signboard::setDistance(int distance)
{
    m_distance = distance;
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

    // a. 将坐标系移动到告示牌的中心点
    painter->translate(m_position);
    painter->rotate(m_rotation);
    painter->scale(m_scale, m_scale); // 应用缩放，确保文字大小正确

    // b. 设置文字的字体、颜色和对齐方式
    QFont font("Impact", 20); // 您可以修改字体和大小
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::white); // 设置文字颜色为白色

    // c. 定义文字绘制的区域 (在告示牌的中心)
    QRectF textRect(-m_originalPixmap.width() / 2, -m_originalPixmap.height()*1.25,
                    m_originalPixmap.width(), m_originalPixmap.height());

    // d. 格式化并绘制文字
    QString distanceText = QString("%1 m").arg(m_distance);
    painter->drawText(textRect, Qt::AlignCenter, distanceText);

    QPen debugPen(Qt::blue); // 用蓝色以作区分
    debugPen.setWidth(3);
    painter->setPen(debugPen);
    painter->drawPath(getBoardPath());
    painter->restore();
}
