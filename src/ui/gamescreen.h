#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>
class QTimer;
class GameObject; // 前向声明
class Player;     // 前向声明
class Obstacle; // <-- 前向声明障碍物类
class House; // <-- 前向声明 House

class GameScreen : public QWidget
{
    Q_OBJECT

public:
    explicit GameScreen(QWidget *parent = nullptr);
    void startGame();
    void stopGame();

protected:
    void paintEvent(QPaintEvent *event) override;
    // --- 新增：声明键盘事件处理函数 ---
    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void updateGame();
    void updateSnow();
private:
    void generateInitialTerrain();
    void placePlayer();
    // --- 新增：放置障碍物和检测碰撞的函数 ---
    void setupObstacles();
    void checkCollisions();
    // --- 新增：检查物体经过触发器的函数 ---
    void checkTriggers();
    QPair<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
    void updateSnowPath();
    // 【核心新增】一个专门用于分析屋顶路径的辅助函数
    QPair<QPointF, qreal> getPathInfoAt(const QPainterPath& path, qreal x_pos);

    QTimer* m_timer;
    QPixmap m_backgroundPixmap;
    qreal m_backgroundOffset;

    // --- 新增：世界滚动偏移量 ---
    qreal m_worldOffset;      // 代表摄像机的位置或者说世界滚动的距离


    // 雪地相关
    QList<QPointF> m_snowPoints;
    QPainterPath m_snowPath;
    // 游戏实体
    Player* m_player;
    // --- 新增：用于存放所有障碍物的列表 ---
    QList<Obstacle*> m_obstacles;
    QList<House*> m_houses; // <-- 为房屋创建一个专门的列表
};

#endif // GAMESCREEN_H
