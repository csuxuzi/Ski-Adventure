#include "game/GameObject.h"
#include <QTransform>

GameObject::GameObject(QObject *parent)
    : QObject(parent), m_rotation(0), m_scale(1.0)
{
}

void GameObject::draw(QPainter *painter)
{
    if (m_pixmap.isNull()) return;

    painter->save(); // 保存当前坐标系状态

    // 1. 将坐标系原点移动到物体的位置（底部中心）
    painter->translate(m_position);
    // 2. 将坐标系旋转到物体的角度
    painter->rotate(m_rotation);

    // 3. 绘制图像，图像的左上角需要向上和向左偏移
    QPointF drawPos(-m_pixmap.width() / 2.0, -m_pixmap.height());
    painter->drawPixmap(drawPos, m_pixmap);

    painter->restore(); // 恢复坐标系状态
}

// 计算并返回在世界坐标系中的实际碰撞体积
QRectF GameObject::collisionRect() const
{
    // 创建一个变换矩阵
    QTransform transform;
    // 首先移动到物体的位置
    transform.translate(m_position.x(), m_position.y());
    // 然后进行旋转
    transform.rotate(m_rotation);

    // 返回经过变换后的碰撞矩形
    return transform.mapRect(m_collisionRect);
}


void GameObject::setScale(qreal scale)
{
    if (m_originalPixmap.isNull()) return;
    // 【核心新增】保存当前的缩放值
    m_scale = scale;
    // 基于原始图像进行缩放，避免精度损失
    m_pixmap = m_originalPixmap.scaled(m_originalPixmap.size() * scale,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);

    // 同步更新碰撞体积
    m_collisionRect = QRectF(-m_pixmap.width() / 2.0, -m_pixmap.height(),
                             m_pixmap.width(), m_pixmap.height());
}
