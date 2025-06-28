#ifndef STONE_H
#define STONE_H

#include "game/Obstacle.h"

class QTimer;

class Stone : public Obstacle
{
    Q_OBJECT

public:
    // 定义石头的两种尺寸
    enum StoneSize {
        Small,
        Large
    };



    // 定义石头的状态
    enum StoneState {
        Intact,      // 完好无损
        FadingOut,   // 正在淡出
        Destroyed    // 已被摧毁
    };

    explicit Stone(StoneSize size, QObject *parent = nullptr);

    void update() override;
    // 【核心】重写基类的draw方法以实现自定义绘制效果
    void draw(QPainter* painter) override;

    // 外部调用的接口，用于触发破碎
    void shatter();

    StoneState currentState;

private slots:
    // 用于更新淡出效果的槽函数
    void fadeOut();

private:
    QPixmap m_shatteredPixmap; // 破碎后的图片
    QTimer* m_fadeTimer;       // 驱动淡出效果的计时器
    qreal m_opacity;           // 当前的透明度 (1.0 -> 0.0)
};

#endif // STONE_H
