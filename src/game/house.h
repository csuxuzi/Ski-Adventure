#ifndef HOUSE_H
#define HOUSE_H

#include "game/Obstacle.h"
#include <QPainterPath>

class House : public Obstacle
{
    Q_OBJECT

public:
    enum HouseState {
        Closed,
        Open
    };

    explicit House(QObject *parent = nullptr);

    // --- 核心：返回屋顶的精确碰撞路径 ---
    QPainterPath getRoofPath() const;

    // --- 状态控制 ---
    void openDoor();
    // (未来可以添加 shatter() 函数)

    // --- 重写绘制函数以添加门 ---
    void draw(QPainter* painter) override;

private:
    HouseState m_doorState;
    QPixmap m_doorPixmap;
    QPainterPath m_roofPath; // 用于存储屋顶的形状
};

#endif // HOUSE_H
