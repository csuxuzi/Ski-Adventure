#ifndef SIGNBOARD_H
#define SIGNBOARD_H

#include "game/Obstacle.h"
#include <QPainterPath>

class QTimer;

class Signboard : public Obstacle
{
    Q_OBJECT

public:
    enum ShatterState {
        Intact,
        Shattering,
        Shattered
    };

    explicit Signboard(QObject *parent = nullptr);

    void setDistance(int distance);//显示滑行距离
    QPainterPath getBoardPath() const;
    void shatter(const QPointF& point);
    void draw(QPainter* painter) override;
    ShatterState currentState() const { return m_shatterState; }

private slots:
    void updateShatterEffect();

private:
    QPainterPath m_boardPath;
    ShatterState m_shatterState;
    QPixmap m_shatteredPixmap;
    QTimer* m_shatterTimer;
    qreal m_shatterOpacity;
    QPointF m_shatterPosition;
    int m_distance; // 用于存储距离
};

#endif // SIGNBOARD_H
