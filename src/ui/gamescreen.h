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
class Seesaw;
class Signboard;
class PauseDialog; // <-- 【新增】前向声明 PauseDialog
class ImageButton; // <-- 【新增】前向声明 ImageButton
class Avalanche;
class GameOverDialog;
class Mount;   // <-- 【新增】
class Penguin; // <-- 【新增】
class Yeti;    // <-- 【新增】


class GameScreen : public QWidget
{
    Q_OBJECT

public:
    explicit GameScreen(QWidget *parent = nullptr);
    void startGame();
    void stopGame();

    // 【新增】一个 public 的 getTerrainInfoAt, 以便雪崩对象可以调用
    QPair<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
signals:
    // 【新增】定义一个信号，用于通知主窗口返回主菜单
    void backToMainMenuRequested();
protected:
    void paintEvent(QPaintEvent *event) override;
    // --- 新增：声明键盘事件处理函数 ---
    void keyPressEvent(QKeyEvent *event) override;
private slots:
    void updateGame();
    void updateSnow();
    // --- 【新增】处理暂停逻辑的槽函数 ---
    void onPauseButtonClicked();

    void handleExit();

public slots:
    void restartGame();
private:
    void setupUI(); // <-- 【新增】一个专门设置UI的函数
    void generateInitialTerrain();
    // void placePlayer();
    // --- 【修改】重命名旧的 placePlayer 为 resetGameSate ---
    void resetGameState();
    // --- 新增：放置障碍物和检测碰撞的函数 ---
    void setupObstacles();
    void checkCollisions();
    // --- 新增：检查物体经过触发器的函数 ---
    void checkTriggers();
    //<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
    void updateSnowPath();

    // --- 【新增】三种地形的生成函数 ---
    void generateGentleSlope(QList<QPointF>& points, const QPointF& startPoint);
    void generateSteepSlope(QList<QPointF>& points, const QPointF& startPoint);
    void generateCliff(QList<QPointF>& points, const QPointF& startPoint);

    //检测坐骑的碰撞
    template<typename MountType>
    // 修改后
    void checkObstacleCollisionForMount(MountType* mount, qreal initialSpeed);

    // 【核心新增】一个专门用于分析屋顶路径的辅助函数
    QPair<QPointF, qreal> getPathInfoAt(const QPainterPath& path, qreal x_pos);
    // --- 【新增】计算撞击角度的辅助函数 ---
    qreal calculateImpactAngle(const QVector2D& velocity, qreal surfaceAngleDegrees) const;

    QTimer* m_timer;
    QPixmap m_backgroundPixmap;
    qreal m_backgroundOffset;

    // --- 新增：世界滚动偏移量 ---
    qreal m_worldOffset;      // 代表摄像机的位置或者说世界滚动的距离
    qreal m_verticalOffset;   // <-- 【新增】垂直偏移量

    // 雪地相关
    QList<QPointF> m_snowPoints;
    QPainterPath m_snowPath;

    // --- 【新增】用于暂存下一个地形块的点 ---
    QList<QPointF> m_nextTerrainSegment;
    // --- 【新增】记录障碍物已生成到的最远X坐标 ---
    qreal m_lastObstacleX;

    // 游戏实体
    Player* m_player;
    Avalanche* m_avalanche; // <-- 【新增】雪崩对象指针
    // --- 新增：用于存放所有障碍物的列表 ---
    QList<Obstacle*> m_obstacles;
    QList<House*> m_houses; // <-- 为房屋创建一个专门的列表
    QList<Seesaw*> m_seesaws; // <-- 【新增】为翘板创建一个专门的列表
    QList<Signboard*> m_signboards;
    // --- 【新增】暂停相关的UI成员 ---
    ImageButton* m_pauseButton;
    PauseDialog* m_pauseDialog;
    GameOverDialog* m_gameOverDialog; // <-- 【新增】
    QList<Penguin*> m_penguins; // <-- 【新增】
    QList<Yeti*> m_yetis;       // <-- 【新增】
};

#endif // GAMESCREEN_H
