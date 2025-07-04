#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>
#include <QQueue>
#include <QLabel>
class QTimer;
class GameObject;
class Player;
class Obstacle;
class House;
class Seesaw;
class Signboard;
class PauseDialog;
class ImageButton;
class Avalanche;
class GameOverDialog;
class Tree;
class CardSelectionDialog;
class Mount;
class Penguin;
class Yeti;
class Coin;

class GameScreen : public QWidget
{
    Q_OBJECT

public:
    enum TerrainType {
        Gentle,
        Steep,
        Cliff
    };

    explicit GameScreen(QWidget *parent = nullptr);
    void startGame();
    void stopGame();
    void addScore(int baseScore);
    ///卡片相关
    Player* getPlayer() const { return m_player; }
    Avalanche* getAvalanche() const { return m_avalanche; }
    void applyScoreMultiplier(float multiplier, int durationMs);

    // 用于获取雪地y轴的接口
    QPair<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
signals:
    // 定义一个信号，用于通知主窗口返回主菜单
    void backToMainMenuRequested();
protected:
    void paintEvent(QPaintEvent *event) override;
    // 声明键盘事件处理函数
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;
private slots:
    void updateGame();
    void updateSnow();
    // 处理暂停逻辑的槽函数
    void onPauseButtonClicked();

    void handleExit();

public slots:
    void restartGame();
private:
    void setupUI(); // 一个专门设置UI的函数
    void generateInitialTerrain();
    // void placePlayer();
    // 重命名旧的 placePlayer 为 resetGameSate
    void resetGameState();
    // 放置障碍物和检测碰撞的函数
    void setupObstacles();
    void setupMounts();
    void checkCollisions();
    // 检查物体经过触发器的函数
    void checkTriggers();
    //<QPointF, qreal> getTerrainInfoAt(qreal x_pos);
    void updateSnowPath();

    void showDebugInfo();

    // 根据概率选择并生成下一个地形模式的函数
    void generateNextPattern();

    // 三种地形的生成函数
    void generateGentleSlope(QList<QPointF>& points, const QPointF& startPoint);
    void generateSteepSlope(QList<QPointF>& points, const QPointF& startPoint);
    void generateCliff(QList<QPointF>& points, const QPointF& startPoint);


    // 新的素材放置总管函数
    void placeObjectsForSegment(TerrainType type, const QList<QPointF>& segmentPoints);
    void generateMountsInWindow(qreal windowStartX);


    // 辅助函数
    TerrainType getTerrainTypeAt(qreal x_pos);
    bool isLastSegmentOfPattern(qreal x_pos);


    //检测坐骑的碰撞
    template<typename MountType>
    void checkObstacleCollisionForMount(MountType* mount, qreal initialSpeed);

    // 一个专门用于分析屋顶路径的辅助函数
    QPair<QPointF, qreal> getPathInfoAt(const QPainterPath& path, qreal x_pos);
    // 计算撞击角度的辅助函数
    qreal calculateImpactAngle(const QVector2D& velocity, qreal surfaceAngleDegrees) const;

    QTimer* m_timer;
    QPixmap m_backgroundPixmap;
    qreal m_backgroundOffset;

    // 世界滚动偏移量
    qreal m_worldOffset;      // 代表摄像机的位置或者说世界滚动的距离
    qreal m_verticalOffset;   // 垂直偏移量

    // 雪地相关
    QList<QPointF> m_snowPoints;
    QPainterPath m_snowPath;

    // 用于暂存下一个地形块的点
    QList<QPointF> m_nextTerrainSegment;
    // 记录障碍物已生成到的最远X坐标
    qreal m_lastObstacleX;

    // 模式化地形生成相关
    QQueue<TerrainType> m_terrainPatternQueue; // 存储当前模式的地形序列
    qreal m_probNormal;         // “一般”模式的概率
    qreal m_probExciting;       // “爽”模式的概率
    qreal m_probSuperExciting;  // “超爽”模式的概率


    // 素材概率控制变量
    qreal m_probBigStone;
    qreal m_probHouse;
    qreal m_probSleighInHouse;
    qreal m_probSeesawOnCliff;
    qreal m_probSeesawOnSteep;
    qreal m_probPenguin_1;
    qreal m_probPenguin_2;
    qreal m_probPenguin_3;
    qreal m_probYeti_1;
    qreal m_probYeti_2;

    // 素材生成追踪变量
    qreal m_lastObstacleGenX; // 用于障碍物窗口的生成
    qreal m_lastMountGenX;    // 用于坐骑窗口的生成
    qreal m_lastSignboardGenX;
    qreal m_lastMountX = 0;;

    int m_signboardCount; // 用于计算已生成了多少个告示牌
    int m_nextSignboardDistance; // 记录下一个告示牌的距离

    // 游戏实体
    Player* m_player;
    Avalanche* m_avalanche; // 雪崩对象指针
    // 用于存放所有障碍物的列表
    QList<Obstacle*> m_obstacles;
    QList<House*> m_houses; // 为房屋创建一个专门的列表
    QList<Seesaw*> m_seesaws; // 为翘板创建一个专门的列表
    QList<Signboard*> m_signboards;
    // 暂停相关的UI成员
    ImageButton* m_pauseButton;
    PauseDialog* m_pauseDialog;
    GameOverDialog* m_gameOverDialog;
    CardSelectionDialog* m_cardDialog;
    QLabel* m_infoLabel;          // 用于显示速度和距离的标签
    ImageButton* m_debugButton;    // 用于打开调试窗口的按钮


    QList<Penguin*> m_penguins;
    QList<Yeti*> m_yetis;
    QList<Coin*> m_coins;

    // 分数倍率
    qreal m_scoreMultiplier;
    // 将 m_scoreMultiplier 移动到这里，并添加一个计时器
    QTimer* m_scoreMultiplierTimer; // 用于恢复得分倍率
    // 得分系统相关成员
    QLabel* m_scoreLabel;                  // 用于显示分数的UI标签
    quint64 m_score;                       // 存储当前总分 (使用 quint64 防止溢出)
    qreal m_distanceTraveledForScore;      // 记录自上次加分后，玩家移动的距离

    // 一个列表来管理所有的树
    QList<Tree*> m_trees;
    // 一个变量来记录上次种树的位置
    qreal m_lastTreeX;
    // 雪崩警告特效相关
    QPixmap m_warningPixmap;        // 警告图片
    bool m_isWarningVisible;        // 图片当前是否可见
    qreal m_warningScale;           // 图片当前的缩放比例
    qreal m_warningRotation;        // 图片当前的旋转角度
    qreal m_rotationDirection;      // 旋转方向 (1 或 -1)
    QTimer* m_warningDelayTimer; // 换成这个可以完全控制的定时器

    bool m_warningEnabled;
};

#endif // GAMESCREEN_H
