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
        Falling,  // 下落
        Crashing,
        StandingUp // 新增：站起来的过程中
    };

    explicit Player(QObject *parent = nullptr);

    // 覆盖基类的 update 方法来实现玩家自己的逻辑
    void update() override;

    // --- 公共状态变量 ---
    PlayerState currentState = Falling; // 玩家初始状态为下落
    bool onGround = false;              // 是否在地面上
    bool isInvincible = false; // <-- 【新增】无敌状态标志

    // --- 跳跃控制 ---
    void jump(); // 执行跳跃的函数
    void crash();
    void reduceCrashTime(); // <-- 【新增】响应键盘点击，减少摔倒时间

private slots:
    // --- 新增：用于更新动画帧的槽函数 ---
    void updateAnimation();
    void handleCrashTimeout(); // <-- 【新增】处理5秒摔倒时间结束的槽
private:
    // --- 新增：动画相关成员 ---
    QTimer* m_animationTimer;     // 动画计时器
    int m_currentFrameIndex;      // 当前播放的帧索引

    // 存放不同状态的动画帧
    QList<QPixmap> m_slidingFrames;
    QList<QPixmap> m_jumpingFrames;
    QList<QPixmap> m_fallingFrames;
    QList<QPixmap> m_crashingFrames; // <-- 【新增】
    QList<QPixmap> m_standingUpFrames; // <-- 【新增】站立动画帧列表

    // --- 新增：根据当前状态切换动画的辅助函数 ---
    void setCurrentAnimation(PlayerState state);

    // --- 摔倒逻辑相关成员 ---
    QTimer* m_crashStateTimer;       // <-- 【新增】用于计算5秒摔倒时间的计时器
    qreal m_crashTimeRemaining;      // <-- 【新增】剩余摔倒时间
    int m_crashInitialLoopCount;   // <-- 【新增】记录前5帧播放次数
};

#endif // PLAYER_H
