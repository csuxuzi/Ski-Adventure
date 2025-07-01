#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QObject>
#include <QPointF>
#include <QVector2D>
#include <QPixmap>
#include <QPainter>
#include <QRectF>

class GameObject : public QObject
{
    Q_OBJECT

public:
    explicit GameObject(QObject *parent = nullptr);

    // 核心物理属性
    void setPosition(const QPointF& position) { m_position = position; }
    QPointF position() const { return m_position; }

    void setVelocity(const QVector2D& velocity) { m_velocity = velocity; }
    QVector2D velocity() const { return m_velocity; }

    void setRotation(qreal rotation) { m_rotation = rotation; }
    qreal rotation() const { return m_rotation; }

    QRectF collisionRect() const;
    // 设置缩放比例
    void setScale(qreal scale);
    // 让GameObject能返回自己的缩放比例
    qreal scale() const { return m_scale; }
    // 核心行为
    // 纯虚函数，子类必须实现具体的更新逻辑
    virtual void update() = 0;

    // 将draw函数声明为虚函数，允许子类重写
    virtual void draw(QPainter* painter);

protected:
    QPointF m_position;      // 位置 (参照点为底部中心)
    QVector2D m_velocity;    // 速度矢量
    qreal m_rotation;        // 角度 (角度制)
    QPixmap m_pixmap;        // 图像
    QPixmap m_originalPixmap;     // 原始未经缩放的图像
    QRectF m_collisionRect;  // 碰撞体积
    // 增加一个成员变量来存储缩放比例
    qreal m_scale;
};

#endif // GAMEOBJECT_H
