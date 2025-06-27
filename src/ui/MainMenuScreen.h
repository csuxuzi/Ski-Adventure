#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H

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
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;

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
};


#endif // MAINMENUSCREEN_H
