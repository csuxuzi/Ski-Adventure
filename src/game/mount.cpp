#include "game/Mount.h"
#include <QTimer>
#include <QPainter> // 需要包含 QPainter

// 定义动画常量
const int FADE_STEP_DURATION = 50;
const qreal OPACITY_STEP = 0.08;

Mount::Mount(QObject *parent)
    : GameObject(parent),
    currentState(Idle),
    onGround(false),
    m_animationTimer(nullptr),
    m_currentFrameIndex(0),
    m_disappearState(Intact), // 【新增】初始化状态
    m_opacity(1.0)          // 【新增】初始化透明度
{
    // 【新增】创建并连接用于淡出动画的计时器
    m_fadeTimer = new QTimer(this);
    connect(m_fadeTimer, &QTimer::timeout, this, &Mount::updateFadeOut);
}


// 【新增】触发消失动画的实现
void Mount::disappear()
{
    if (m_disappearState == Intact) {
        m_disappearState = FadingOut;
        m_animationTimer->stop(); // 停止移动动画
        m_fadeTimer->start(FADE_STEP_DURATION);
    }
}

// 【新增】更新淡出效果的槽函数实现
void Mount::updateFadeOut()
{
    m_opacity -= OPACITY_STEP;
    if (m_opacity <= 0.0) {
        m_opacity = 0.0;
        m_fadeTimer->stop();
        m_disappearState = Gone;
        emit disappeared(); // 发射信号
        this->deleteLater(); // 动画结束，自我销毁
    }
}

// 【新增】重写的 draw 函数
void Mount::draw(QPainter* painter)
{
    painter->save();

    if (m_disappearState == FadingOut) {
        // 如果正在消失，设置透明度
        painter->setOpacity(m_opacity);
    }

    // 调用基类的 draw 来绘制坐骑本身（带或不带透明度）
    GameObject::draw(painter);

    // 如果正在消失，额外在中心绘制“烟雾”效果
    if (m_disappearState == FadingOut && !m_disappearPixmap.isNull()) {
        QPointF drawPos = m_position - QPointF(m_disappearPixmap.width() / 2.0, m_disappearPixmap.height() / 2.0);
        painter->drawPixmap(drawPos, m_disappearPixmap);
    }

    painter->restore();
}


// 默认的动画更新逻辑，循环播放 m_movingFrames
void Mount::updateAnimation()
{
    if (!m_movingFrames.isEmpty()) {
        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_movingFrames.size();
        m_originalPixmap = m_movingFrames[m_currentFrameIndex];
        // 确保应用了正确的缩放
        setScale(this->scale());
    }
}
