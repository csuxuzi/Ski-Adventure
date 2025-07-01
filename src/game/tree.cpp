#include "game/Tree.h"
#include <QDebug>
#include <QPainter>
Tree::Tree(QObject *parent) : GameObject(parent)
{
    // 在这里加载您的树木图片
    if (!m_originalPixmap.load(":/assets/images/tree.png")) {
        qWarning() << "没找到树的图片啊！tree.png";
    }
    setScale(0.8);
}

void Tree::update()
{
}
void Tree::draw(QPainter* painter)
{
    painter->save(); // 保存一下画笔当前的状态
    // 将坐标系平移到这棵树自己的位置
    painter->translate(m_position);
    painter->setOpacity(0.5);
    if (!m_pixmap.isNull()) {
        QPointF drawPos(-m_pixmap.width() / 2.0, -m_pixmap.height());
        painter->drawPixmap(drawPos, m_pixmap);
    }
    painter->restore(); // 恢复画笔状态，不影响其他东西的绘制
}
