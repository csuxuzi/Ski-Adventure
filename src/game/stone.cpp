#include "game/Stone.h"
#include <QTimer>
#include <QDebug>
#include "audio/AudioManager.h"
// 破碎图片显示的时间 (毫秒)
const int SHATTER_DURATION = 500;
// 淡出效果每步之间的时间间隔 (毫秒)
const int FADE_STEP_DURATION = 50;
// 每次降低多少透明度
const qreal OPACITY_STEP = 0.05;
Stone::Stone(StoneSize size, QObject *parent)
    : Obstacle(parent), currentState(Intact), m_opacity(1.0)
{
    // 1. 根据尺寸加载不同的石头图片
    QString imagePath;
    if (size == Large) {
        imagePath = ":/assets/images/stone_large1.png"; // 请替换为大石头的图片路径
    } else {
        imagePath = ":/assets/images/stone_small1.png"; // 请替换为小石头的图片路径
    }


    if (!m_originalPixmap.load(imagePath)) {
        qWarning() << "Failed to load stone image:" << imagePath;
    }

    setScale(0.5); // 初始化碰撞体积和 m_pixmap

    // 2. 加载破碎后的图片
    if (!m_shatteredPixmap.load(":/assets/images/stone_shattered.png")) { // 请替换为您的破碎图片路径
        qWarning() << "Failed to load shattered stone image!";
    }

    // 3. 初始化计时器
    m_fadeTimer = new QTimer(this);
    connect(m_fadeTimer, &QTimer::timeout, this, &Stone::fadeOut);
}

void Stone::shatter()
{
    if (currentState == Intact) {
        currentState = FadingOut;
        // 【新增】播放石头破碎音效
        AudioManager::instance()->playSoundEffect(SfxType::StoneShatter);
        m_fadeTimer->start(FADE_STEP_DURATION); // 启动淡出计时器
    }
}

// 这个槽函数会被周期性调用
void Stone::fadeOut()
{
    m_opacity -= OPACITY_STEP; // 降低透明度

    if (m_opacity <= 0.0) {
        m_opacity = 0.0;
        m_fadeTimer->stop();   // 停止计时器
        currentState = Destroyed;   // 标记为“已被摧毁”
    }
}

// 【核心】自定义的绘制函数
void Stone::draw(QPainter* painter)
{
    // 如果石头是完好的，就使用基类的默认绘制方法
    if (currentState == Intact) {
        GameObject::draw(painter);
        return;
    }

    // 如果石头正在淡出或已摧毁，则绘制带透明度的破碎贴图
    if (m_opacity > 0.0) {
        painter->save(); // 保存当前的 painter 状态
        painter->setOpacity(m_opacity); // 设置透明度

        // 在石头原来的位置绘制破碎贴图
        // 我们需要计算绘制的左上角坐标
        QPointF drawPos = m_position - QPointF(m_shatteredPixmap.width() / 2.0, m_shatteredPixmap.height());
        painter->drawPixmap(drawPos, m_shatteredPixmap);

        painter->restore(); // 恢复 painter 状态，避免影响其他物体
    }
}

void Stone::update() { /* 无需代码 */ }
