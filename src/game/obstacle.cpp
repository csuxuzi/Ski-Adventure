#include "game/Obstacle.h"

Obstacle::Obstacle(QObject *parent) : GameObject(parent)
{
}

// 目前静态障碍物不需要每帧更新自己的状态
void Obstacle::update()
{
    // 空实现
}
