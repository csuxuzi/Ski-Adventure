#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QObject>
#include <QPointF>
#include <QVector2D>
#include <QPixmap>
#include <QPainter>

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

    // 核心行为
    virtual void update() = 0; // 纯虚函数，子类必须实现
    void draw(QPainter* painter);

protected:
    QPointF m_position;
    QVector2D m_velocity;
    qreal m_rotation; // 角度制
    QPixmap m_pixmap;
};

#endif // GAMEOBJECT_H
