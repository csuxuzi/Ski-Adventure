#ifndef PLAYER_H
#define PLAYER_H

#include "game/GameObject.h"
#include <QList>
#include <QPixmap>

class QTimer; // 前向声明

class Player : public GameObject
{
    Q_OBJECT

public:
    // 定义玩家的几种物理状态
    enum PlayerState {
        Sliding, // 在地面滑行
        Jumping, // 起跳和上升
        Falling  // 下落
    };

    explicit Player(QObject *parent = nullptr);

    // 覆盖基类的 update 方法来实现玩家自己的逻辑
    void update() override;

    // --- 公共状态变量 ---
    PlayerState currentState = Falling; // 玩家初始状态为下落
    bool onGround = false;              // 是否在地面上

    // --- 跳跃控制 ---
    void jump(); // 执行跳跃的函数

private slots:
    // --- 新增：用于更新动画帧的槽函数 ---
    void updateAnimation();

private:
    // --- 新增：动画相关成员 ---
    QTimer* m_animationTimer;     // 动画计时器
    int m_currentFrameIndex;      // 当前播放的帧索引

    // 存放不同状态的动画帧
    QList<QPixmap> m_slidingFrames;
    QList<QPixmap> m_jumpingFrames;
    QList<QPixmap> m_fallingFrames;

    // --- 新增：根据当前状态切换动画的辅助函数 ---
    void setCurrentAnimation(PlayerState state);
};

#endif // PLAYER_H
