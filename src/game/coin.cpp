#include "game/Coin.h"
#include <QTimer>
#include <QDebug>

const int COIN_ANIMATION_FRAME_RATE = 100;

Coin::Coin(QObject *parent)
    : GameObject(parent), m_currentFrame(0)
{
    // 加载金币旋转的动画帧
    for (int i = 1; i <= 16; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/coins/coin_%1.png").arg(i);
        if (frame.load(path)) {
            m_frames.append(frame);
        } else {
            qWarning() << "Failed to load coin frame:" << path;
        }
    }

    // 初始化动画系统
    if (!m_frames.isEmpty()) {
        m_originalPixmap = m_frames.first();
        setScale(0.6);
    }

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Coin::updateAnimation);
    m_animationTimer->start(COIN_ANIMATION_FRAME_RATE);
}

void Coin::update()
{

}

// 循环播放动画帧
void Coin::updateAnimation()
{
    if (!m_frames.isEmpty()) {
        m_currentFrame = (m_currentFrame + 1) % m_frames.size();
        m_originalPixmap = m_frames[m_currentFrame];
        setScale(this->scale());
    }
}
