#ifndef TREE_H
#define TREE_H

#include "game/GameObject.h"

class Tree : public GameObject
{
    Q_OBJECT
public:
    explicit Tree(QObject *parent = nullptr);

    // 树是静态的，不需要每帧更新，但需要保留这个空函数
    void update() override;
};

#endif // TREE_H
