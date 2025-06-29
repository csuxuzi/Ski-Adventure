#include "game/Mount.h"
#include <QTimer>

Mount::Mount(QObject *parent)
    : GameObject(parent),
    currentState(Idle),
    onGround(false),
    m_animationTimer(nullptr),
    m_currentFrameIndex(0)
{
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
