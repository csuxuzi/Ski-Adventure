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
    // 定义玩家可骑乘的坐骑类型
    enum MountType {
        None,
        Penguin,
        Yeti,
        BrokenYeti,
        Sleigh
    };

    // 定义玩家的几种物理状态
    enum PlayerState {
        Sliding, // 在地面滑行
        Jumping, // 起跳和上升
        Falling,  // 下落
        Crashing,
        StandingUp // 站起来的过程中
    };

    void draw(QPainter* painter) override;
    explicit Player(QObject *parent = nullptr);

    ///卡片相关接口
    void applySpeedBonus(float bonus);
    void applyJumpBonus(float bonus);
    static const qreal PLAYER_SPEED_MULTIPLIER_ON_BROKEN_YETI;

    // 获取当前速度大小的接口
    qreal currentSpeed() const { return m_currentSpeed; }
    qreal baseSpeed() const { return m_baseSpeed; }
    qreal currentGravity() const { return m_currentGravity; }
    qreal jumpForce() const { return m_jumpForce; }

    // 覆盖基类的 update 方法来实现玩家自己的逻辑
    void update() override;
    void startInvincibility(int customDuration = 0); // customDuration = 0 表示使用默认时间

    // 骑乘系统的公共接口
    void rideMount(MountType type, const QList<QPixmap>& mountFrames, qreal newSpeed, qreal newGravity);
    bool isMounted() const { return m_currentMount != None; }
    MountType currentMountType() const { return m_currentMount; }

    // 公共状态变量
    PlayerState currentState = Falling; // 玩家初始状态为下落
    bool onGround = false;              // 是否在地面上
    bool isInvincible = false;

    // 跳跃控制
    void jump(); // 执行跳跃的函数
    void crash();
    void reduceCrashTime(); // 响应键盘点击，减少摔倒时间

private slots:
    // 用于更新动画帧的槽函数
    void updateAnimation();
    void handleCrashTimeout(); // 处理5秒摔倒时间结束的槽
private:
    // 动画相关成员
    QTimer* m_animationTimer;     // 动画计时器
    int m_currentFrameIndex;      // 当前播放的帧索引
    // 用于控制坐骑动画速度的计数器
    int m_mountAnimCounter;

    // 存放不同状态的动画帧
    QList<QPixmap> m_slidingFrames;
    QList<QPixmap> m_jumpingFrames;
    QList<QPixmap> m_fallingFrames;
    QList<QPixmap> m_crashingFrames;
    QList<QPixmap> m_standingUpFrames; // 站立动画帧列表

    QPixmap m_ridingYetiPixmap;      // 存放骑乘雪怪的静态图
    QPixmap m_ridingBrokenYetiPixmap; // 存放损坏雪怪的静态图
    QPixmap m_ridingPenguinPixmap;   // 存放骑乘企鹅的静态图

    // 坐骑动画相关成员
    QList<QPixmap> m_mountAnimationFrames; // 存储当前所骑坐骑的动画帧
    int m_mountFrameIndex;                 // 当前坐骑动画播放的帧索引

    // 骑乘和物理状态
    MountType m_currentMount;      // 当前骑乘的坐骑类型
    qreal m_currentGravity;        // 当前生效的重力（会根据坐骑改变）

    // 根据当前状态切换动画的辅助函数
    void setCurrentAnimation(PlayerState state);


    // 摔倒逻辑相关成员
    QTimer* m_crashStateTimer;       // 用于计算5秒摔倒时间的计时器
    qreal m_crashTimeRemaining;      // 剩余摔倒时间
    int m_crashInitialLoopCount;   // 记录前5帧播放次数
    // 将无敌时间定义为常量，方便调整
    int INVINCIBILITY_DURATION_MS; // 默认1.5秒
    // 速度控制相关成员
    qreal m_baseSpeed;      // 角色的基础速度，用于重置
    qreal m_currentSpeed;   // 角色当前的实际速度大小
    qreal m_jumpForce; // 用于存储当前跳跃力

    // 无敌光环特效相关
    QPixmap m_invincibleEffectPixmap; // 光环的图片
    qreal m_invincibleEffectOpacity;  // 光环当前的透明度 (0.0 到 1.0)
    bool m_isFadingIn;                // 一个开关，true代表正在淡入，false代表正在淡出
};

#endif // PLAYER_H
