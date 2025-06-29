#include "game/Yeti.h"
#include <QTimer>
#include <QDebug>

const int YETI_ANIMATION_FRAME_RATE = 30; // 雪怪动画帧率 (ms)

Yeti::Yeti(QObject *parent)
    : Mount(parent), m_gravity(0.8) // 雪怪的重力设为0.8，比企鹅重
{
    // 1. 加载雪怪的移动动画帧
    // 【注意】您需要准备好 yeti_1.png, yeti_2.png 等图片
    for (int i = 1; i <= 16; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/yeti/yeti_%1.png").arg(i);
        if (frame.load(path)) {
            m_movingFrames.append(frame);
        } else {
            qWarning() << "Failed to load yeti frame:" << path;
        }
    }

    // 2. 初始化动画系统
    if (!m_movingFrames.isEmpty()) {
        m_originalPixmap = m_movingFrames.first();
        setScale(0.9); // 雪怪更大一些
    }

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Yeti::updateAnimation);
    m_animationTimer->start(YETI_ANIMATION_FRAME_RATE);
}

void Yeti::update()
{
    // 应用自己独立的重力
    if (!onGround) {
        m_velocity.setY(m_velocity.y() + m_gravity);
    }
    // 根据速度更新位置
    m_position += m_velocity.toPointF();
}
