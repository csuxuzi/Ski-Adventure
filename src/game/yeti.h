#ifndef YETI_H
#define YETI_H

#include "game/Mount.h"

class Yeti : public Mount
{
    Q_OBJECT

public:
    explicit Yeti(QObject *parent = nullptr);
    void update() override;

private:
    qreal m_gravity; // 雪怪自己的重力
};

#endif // YETI_H
