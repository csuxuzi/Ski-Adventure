#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>
class QTimer;
class GameObject; // 前向声明
class Player;     // 前向声明

class GameScreen : public QWidget
{
    Q_OBJECT

public:
    explicit GameScreen(QWidget *parent = nullptr);
    void startGame();
    void stopGame();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateGame();
    void updateSnow();
private:
    void generateInitialTerrain();
    void placePlayer();
    QPair<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
    void updateSnowPath();


    QTimer* m_timer;
    QPixmap m_backgroundPixmap;
    qreal m_backgroundOffset;

    // 雪地相关
    QList<QPointF> m_snowPoints;
    QPainterPath m_snowPath;
    // 游戏实体
    Player* m_player;
};

#endif // GAMESCREEN_H
