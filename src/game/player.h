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
    // 【新增】定义玩家可骑乘的坐骑类型
    enum MountType {
        None,
        Penguin,
        Yeti,
        Sleigh
        // 未来可在此添加 Sleigh
    };

    // 定义玩家的几种物理状态
    enum PlayerState {
        Sliding, // 在地面滑行
        Jumping, // 起跳和上升
        Falling,  // 下落
        Crashing,
        StandingUp // 新增：站起来的过程中
    };

    void draw(QPainter* painter) override;
    explicit Player(QObject *parent = nullptr);

    // --- 【新增】获取当前速度大小的接口 ---
    qreal currentSpeed() const { return m_currentSpeed; }

    // 覆盖基类的 update 方法来实现玩家自己的逻辑
    void update() override;


    // --- 【新增】骑乘系统的公共接口 ---
    void rideMount(MountType type, const QList<QPixmap>& mountFrames, qreal newSpeed, qreal newGravity);
    bool isMounted() const { return m_currentMount != None; }
    MountType currentMountType() const { return m_currentMount; }

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
    // --- 【新增】用于控制坐骑动画速度的计数器 ---
    int m_mountAnimCounter;

    // 存放不同状态的动画帧
    QList<QPixmap> m_slidingFrames;
    QList<QPixmap> m_jumpingFrames;
    QList<QPixmap> m_fallingFrames;
    QList<QPixmap> m_crashingFrames; // <-- 【新增】
    QList<QPixmap> m_standingUpFrames; // <-- 【新增】站立动画帧列表

    QPixmap m_ridingYetiPixmap;      // 新增：存放骑乘雪怪的静态图
    QPixmap m_ridingPenguinPixmap;   // 新增：存放骑乘企鹅的静态图

    // --- 【新增】坐骑动画相关成员 ---
    QList<QPixmap> m_mountAnimationFrames; // 存储当前所骑坐骑的动画帧
    int m_mountFrameIndex;                 // 当前坐骑动画播放的帧索引

    // --- 【新增】骑乘和物理状态 ---
    MountType m_currentMount;      // 当前骑乘的坐骑类型
    qreal m_currentGravity;        // 当前生效的重力（会根据坐骑改变）

    // --- 新增：根据当前状态切换动画的辅助函数 ---
    void setCurrentAnimation(PlayerState state);

    // --- 摔倒逻辑相关成员 ---
    QTimer* m_crashStateTimer;       // <-- 【新增】用于计算5秒摔倒时间的计时器
    qreal m_crashTimeRemaining;      // <-- 【新增】剩余摔倒时间
    int m_crashInitialLoopCount;   // <-- 【新增】记录前5帧播放次数
    // --- 【新增】速度控制相关成员 ---
    qreal m_baseSpeed;      // 角色的基础速度，用于重置
    qreal m_currentSpeed;   // 角色当前的实际速度大小
};

#endif // PLAYER_H
