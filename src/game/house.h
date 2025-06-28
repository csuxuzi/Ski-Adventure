#ifndef HOUSE_H
#define HOUSE_H

#include "game/Obstacle.h"
#include <QPainterPath>
class QTimer;
class House : public Obstacle
{
    Q_OBJECT

public:
    enum HouseState {
        Closed,
        Open
    };

    // --- 【新增】定义房屋的破碎状态 ---
    enum ShatterState {
        Intact,      // 完好
        Shattering,  // 正在破碎（淡出）
        Shattered    // 已破碎
    };

    explicit House(QObject *parent = nullptr);

    // --- 核心：返回屋顶的精确碰撞路径 ---
    QPainterPath getRoofPath() const;

    // --- 【新增】返回屋顶最高点的世界坐标 ---
    QPointF getRoofPeakWorldPosition() const;
    // --- 【新增】获取屋顶左右底角的世界坐标 ---
    QPointF getRoofLeftCornerWorldPosition() const;
    QPointF getRoofRightCornerWorldPosition() const;
    // --- 状态控制 ---
    void openDoor();
    // (未来可以添加 shatter() 函数)


    // --- 【新增】外部调用的破碎接口 ---
    // 参数 point: 在世界坐标系中的碰撞点，破碎贴图将在此处显示
    void shatter(const QPointF& point);


    // --- 重写绘制函数以添加门 ---
    void draw(QPainter* painter) override;
private slots:
    // --- 【新增】用于更新破碎淡出效果的槽函数 ---
    void updateShatterEffect();

private:
    HouseState m_doorState;
    QPixmap m_doorPixmap;
    QPainterPath m_roofPath; // 用于存储屋顶的形状
    // --- 【新增】用于存储屋顶左右底角的局部坐标 ---
    QPointF m_roofLeftCorner;
    QPointF m_roofRightCorner;
    // --- 【新增】用于存储屋顶最高点的局部坐标 ---
    QPointF m_roofPeak;

    // --- 【新增】破碎效果相关的成员变量 ---
    ShatterState m_shatterState;     // 当前的破碎状态
    QPixmap m_shatteredPixmap;       // 破碎效果的贴图
    QTimer* m_shatterTimer;          // 驱动淡出效果的计时器
    qreal m_shatterOpacity;          // 破碎贴图的当前透明度
    QPointF m_shatterPosition;       // 破碎贴图在世界坐标系中的位置

};

#endif // HOUSE_H
