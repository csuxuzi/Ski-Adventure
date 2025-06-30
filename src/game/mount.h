#ifndef MOUNT_H
#define MOUNT_H

#include <QList>
#include <QPixmap>
#include "game/GameObject.h"
class QTimer;

class Mount : public GameObject // 直接继承自 GameObject，因为它有自己的复杂逻辑
{
    Q_OBJECT

public:
    explicit Mount(QObject *parent = nullptr);

    // 骑乘状态
    enum RidingState {
        Idle,       // 空闲状态，自己在跑
        BeingRidden // 被玩家骑乘
    };

    // 纯虚函数，强制子类实现自己的物理更新
    virtual void update() override = 0;

    // --- 【新增】返回动画帧列表的公共接口 ---
    const QList<QPixmap>& getAnimationFrames() const { return m_movingFrames; }

    // --- 公共状态 ---
    RidingState currentState;
    bool onGround;

protected:
    // 动画相关
    QTimer* m_animationTimer;
    int m_currentFrameIndex;
    QList<QPixmap> m_movingFrames;

protected slots:
    // 动画帧更新的槽函数，子类可以复用
    virtual void updateAnimation();
};

#endif // MOUNT_H
