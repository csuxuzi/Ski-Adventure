#ifndef AVALANCHE_H
#define AVALANCHE_H

#include "game/GameObject.h"

class GameScreen; // 前向声明

class Avalanche : public GameObject
{
    Q_OBJECT

public:
    // 构造函数需要一个指向 GameScreen 的指针，以便获取地形信息
    explicit Avalanche(GameScreen *gameScreen, QObject *parent = nullptr);

    // 雪崩自己的更新逻辑
    void update() override;

    qreal width() const;

private:
    GameScreen* m_gameScreen; // 用于访问地形信息的指针
    qreal m_acceleration;     // X轴的加速度
    qreal m_verticalOffset;   // 相对于地形的垂直高度偏移
};

#endif // AVALANCHE_H
