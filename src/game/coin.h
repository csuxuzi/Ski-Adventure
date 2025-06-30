#ifndef COIN_H
#define COIN_H

#include "game/GameObject.h"

class QTimer;

class Coin : public GameObject
{
    Q_OBJECT

public:
    explicit Coin(QObject *parent = nullptr);

    // 金币的动画效果
    void updateAnimation();
    void update();

private:
    QTimer* m_animationTimer;
    int m_currentFrame;
    QList<QPixmap> m_frames;
};

#endif // COIN_H
