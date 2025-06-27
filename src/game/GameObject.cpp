#include "game/GameObject.h"

GameObject::GameObject(QObject *parent)
    : QObject(parent), m_rotation(0)
{
}

void GameObject::draw(QPainter *painter)
{
    if (m_pixmap.isNull()) return;

    painter->save(); // 保存当前坐标系状态

    // 1. 将坐标系原点移动到物体的位置
    painter->translate(m_position);
    // 2. 将坐标系旋转到物体的角度
    painter->rotate(m_rotation);

    // 3. 绘制图像
    // 我们将图像向上平移其高度，并向左平移其一半宽度
    // 这样，旋转和定位的中心点就是图像的“底部中心”
    QPointF drawPos(-m_pixmap.width() / 2.0, -m_pixmap.height());
    painter->drawPixmap(drawPos, m_pixmap);

    painter->restore(); // 恢复坐标系状态，不影响其他物体绘制
}
