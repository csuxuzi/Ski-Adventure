#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "game/GameObject.h"

class Obstacle : public GameObject
{
    Q_OBJECT

public:
    explicit Obstacle(QObject *parent = nullptr);

    // 静态障碍物通常没有复杂的更新逻辑，但我们保留它以备将来之用
    void update() override;
};

#endif // OBSTACLE_H
