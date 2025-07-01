#include "game/Tree.h"
#include <QDebug>
#include <QPainter>
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
void Tree::draw(QPainter* painter)
{
    painter->save(); // 保存一下画笔当前的状态
    painter->setOpacity(0.7); // 【核心】把透明度设为70%（您也可以改成0.6, 0.8等）
    GameObject::draw(painter); // 调用基类的 draw 函数，把自己（带透明度）画出来
    painter->restore(); // 恢复画笔状态，不影响其他东西的绘制
}
