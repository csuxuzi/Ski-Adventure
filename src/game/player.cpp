#include "game/Player.h"
#include <QDebug>

Player::Player(QObject *parent) : GameObject(parent)
{
    if (!m_pixmap.load(":/assets/images/player.png")) {
        qWarning("Failed to load player image!");
    }
}

void Player::update()
{
    // 根据速度更新位置
    // QPointF 是基于整数的，为了更平滑的移动，我们直接操作 m_position 的成员
    m_position += m_velocity.toPointF();
}
