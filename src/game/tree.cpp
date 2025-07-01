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
    // 2. 首先，将坐标系平移到这棵树自己的位置
    painter->translate(m_position);
    painter->setOpacity(0.5); // 【核心】把透明度设为70%（您也可以改成0.6, 0.8等）
    if (!m_pixmap.isNull()) {
        // 基类 draw 的核心逻辑是这两行，我们直接拿过来用
        QPointF drawPos(-m_pixmap.width() / 2.0, -m_pixmap.height());
        painter->drawPixmap(drawPos, m_pixmap);
    }
    painter->restore(); // 恢复画笔状态，不影响其他东西的绘制
}
