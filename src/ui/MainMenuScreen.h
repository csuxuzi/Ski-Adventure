#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H
#include <QPixmap> // 【新增】包含 QPixmap
#include <QWidget>
#include <QResizeEvent>
class QGraphicsView;
class QGraphicsScene;
class QPropertyAnimation;
class ImageButton;

class MainMenuScreen : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenuScreen(QWidget *parent = nullptr);
    void resetUI(); // <-- 【新增】公开的UI重置函数
signals:
    void startGameClicked();
    void settingsClicked();

private slots:
    void toggleDrawer();
    void showHelp();
    void showRecords();
    void showHighscore();
    //void showSettingsDialog();

private:
    void setupScene();
    void setupUI();
    void startAvalancheAnimation();

    // 场景相关
    // QGraphicsView* m_view;
    // QGraphicsScene* m_scene;
    // --- 【新增】用于存放主界面图片的成员变量 ---
    QPixmap m_characterPixmap;
    QPixmap m_signboardPixmap;
    QPixmap m_postPixmap;
    QPixmap m_gameTitlePixmap;
    QPixmap m_housePixmap;
    // UI 元素
    ImageButton* m_startButton;

    // 抽屉菜单相关
    QWidget* m_drawerContainer;
    ImageButton* m_toggleButton;
    ImageButton* m_helpButton;
    ImageButton* m_recordsButton;
    ImageButton* m_settingsButton;
    ImageButton* m_highscoreButton;
    QPropertyAnimation* m_drawerAnimation;
    bool m_isDrawerOpen = false;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent* event) override;
};


#endif // MAINMENUSCREEN_H
