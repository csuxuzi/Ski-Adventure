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

    // 【新增】定义坐骑的消失状态
    enum DisappearState {
        Intact,     // 完好
        FadingOut,  // 正在淡出
        Gone        // 已消失
    };

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

    // 【新增】触发消失动画的公共接口
    void disappear();
    // 【新增】重写 draw 函数以实现淡出效果
    void draw(QPainter* painter) override;

signals:
    // 【新增】动画播放完毕后，发射此信号通知外界
    void disappeared();

protected:
    // 动画相关
    QTimer* m_animationTimer;
    int m_currentFrameIndex;
    QList<QPixmap> m_movingFrames;

    // 【新增】消失动画相关的成员变量
    DisappearState m_disappearState;
    QPixmap m_disappearPixmap;      // 消失效果的贴图（例如一团烟雾）
    QTimer* m_fadeTimer;            // 驱动淡出效果的计时器
    qreal m_opacity;                // 当前的透明度

protected slots:
    // 动画帧更新的槽函数，子类可以复用
    virtual void updateAnimation();
    // 【新增】更新淡出效果的槽函数
    void updateFadeOut();
};

#endif // MOUNT_H
