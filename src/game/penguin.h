#ifndef PENGUIN_H
#define PENGUIN_H

#include "game/Mount.h"

class Penguin : public Mount
{
    Q_OBJECT

public:
    explicit Penguin(QObject *parent = nullptr);
    void update() override;

private:
    qreal m_gravity; // 企鹅自己的重力加速度
};

#endif // PENGUIN_H
