#include "game/Tree.h"
#include <QDebug>

Tree::Tree(QObject *parent) : GameObject(parent)
{
    // 在这里加载您的树木图片
    if (!m_originalPixmap.load(":/assets/images/tree.png")) {
        qWarning() << "大哥！没找到树的图片啊！tree.png";
    }
    // 您可以根据需要调整树木的默认大小
    setScale(0.8);
}

// 树是纯装饰，所以 update 函数是空的
void Tree::update()
{
    // I'm a tree, I just stand here.
}
