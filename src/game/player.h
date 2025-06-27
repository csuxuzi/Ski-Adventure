#ifndef PLAYER_H
#define PLAYER_H

#include "game/GameObject.h"

class Player : public GameObject
{
    Q_OBJECT

public:
    explicit Player(QObject *parent = nullptr);
    void update() override;
};

#endif // PLAYER_H
