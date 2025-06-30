#include "game/Coin.h"
#include <QTimer>
#include <QDebug>

const int COIN_ANIMATION_FRAME_RATE = 100; // 金币动画播放速度 (ms)

Coin::Coin(QObject *parent)
    : GameObject(parent), m_currentFrame(0)
{
    // 1. 加载金币旋转的动画帧
    //    请确保您已准备好 coin_1.png, coin_2.png 等图片
    for (int i = 1; i <= 16; ++i) { // 假设有6帧动画
        QPixmap frame;
        QString path = QString(":/assets/images/coins/coin_%1.png").arg(i);
        if (frame.load(path)) {
            m_frames.append(frame);
        } else {
            qWarning() << "Failed to load coin frame:" << path;
        }
    }

    // 2. 初始化动画系统
    if (!m_frames.isEmpty()) {
        m_originalPixmap = m_frames.first();
        setScale(0.6); // 您可以在这里调整金币的初始大小
    }

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Coin::updateAnimation);
    m_animationTimer->start(COIN_ANIMATION_FRAME_RATE);
}

// 金币不需要复杂的物理更新
void Coin::update()
{
    // 空实现
}

// 循环播放动画帧
void Coin::updateAnimation()
{
    if (!m_frames.isEmpty()) {
        m_currentFrame = (m_currentFrame + 1) % m_frames.size();
        m_originalPixmap = m_frames[m_currentFrame];
        // 确保应用了正确的缩放
        setScale(this->scale());
    }
}
