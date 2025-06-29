#ifndef SEESAW_H
#define SEESAW_H

#include "game/Obstacle.h"
#include <QPainterPath>

class QTimer;

class Seesaw : public Obstacle
{
    Q_OBJECT

public:
    // 定义翘板的破碎状态
    enum ShatterState {
        Intact,      // 完好
        Shattering,  // 正在破碎（淡出）
        Shattered    // 已破碎
    };

    explicit Seesaw(QObject *parent = nullptr);
    //ShatterState m_shatterState;     // 当前的破碎状态
    // --- 核心：返回翘板的精确碰撞路径 ---
    QPainterPath getPlankPath() const;

    // --- 外部调用的破碎接口 ---
    void shatter(const QPointF& point);
    // --- 【新增】公开的获取状态的方法 ---
    ShatterState currentState() const { return m_shatterState; }
    // --- 重写绘制函数以支持破碎效果 ---
    void draw(QPainter* painter) override;

private slots:
    // 用于更新破碎淡出效果的槽函数
    void updateShatterEffect();

private:
    QPainterPath m_plankPath; // 用于存储翘板的形状

    // --- 破碎效果相关的成员变量 ---

    // --- 【修改】将状态变量设为私有 ---
    ShatterState m_shatterState;
    QPixmap m_shatteredPixmap;
    QTimer* m_shatterTimer;
    qreal m_shatterOpacity;
    QPointF m_shatterPosition;
};

#endif // SEESAW_H
