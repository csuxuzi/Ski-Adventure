#include "game/Penguin.h"
#include <QTimer>
#include <QDebug>

const int PENGUIN_ANIMATION_FRAME_RATE = 100; // 企鹅动画帧率 (ms)

Penguin::Penguin(QObject *parent)
    : Mount(parent), m_gravity(0.5) // 企鹅的重力设为0.5
{
    // 1. 加载企鹅的移动动画帧
    // 【注意】您需要准备好 penguin_1.png, penguin_2.png 等图片
    for (int i = 1; i <= 4; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/penguin/penguin_%1.png").arg(i);
        if (frame.load(path)) {
            m_movingFrames.append(frame);
        } else {
            qWarning() << "Failed to load penguin frame:" << path;
        }
    }

    // 2. 初始化动画系统
    if (!m_movingFrames.isEmpty()) {
        m_originalPixmap = m_movingFrames.first();
        setScale(0.7); // 设置企鹅的尺寸
    }


    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Penguin::updateAnimation);
    m_animationTimer->start(PENGUIN_ANIMATION_FRAME_RATE);
}

void Penguin::update()
{
    // 1. 应用自己独立的重力
    if (!onGround) {
        m_velocity.setY(m_velocity.y() + m_gravity);
    }

    // 2. 根据速度更新位置 (X和Y)
    m_position += m_velocity.toPointF();
}
