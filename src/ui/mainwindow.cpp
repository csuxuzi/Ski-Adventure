#include "ui/MainWindow.h"
#include "ui/MainMenuScreen.h"
#include "ui/SettingsDialog.h"
#include "audio/AudioManager.h"
#include <QStackedWidget>
#include <QDebug>
#include "ui/GameScreen.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/assets/images/app_icon.png"));
    setWindowTitle("Ski Adventure");
    //设置窗口大小：16/9
    setFixedSize(1280, 720);

    setupUI();
    ///链接信号与槽
    createConnections();

    // 启动背景音乐
    // AudioManager::instance()->playMainMenuMusic();
    // 【修改】启动主菜单的背景音乐
    AudioManager::instance()->playBgm(BgmType::MainMenu);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    //堆叠窗口组件，可认为是一叠卡片，每次只显示最上面的一张，用于切换不同的界面
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    //设置主界面
    m_mainMenuScreen = new MainMenuScreen(this);
    m_stackedWidget->addWidget(m_mainMenuScreen);

    m_gameScreen = new GameScreen(this);
    m_stackedWidget->addWidget(m_gameScreen);

    m_settingsDialog = new SettingsDialog(this);

    // 默认显示主菜单
    showMainMenu();
}

void MainWindow::createConnections()
{
    connect(m_mainMenuScreen, &MainMenuScreen::startGameClicked, this, &MainWindow::startGame);
    connect(m_mainMenuScreen, &MainMenuScreen::settingsClicked, this, &MainWindow::showSettings);
    // --- 【新增】连接游戏界面的返回信号 ---
    connect(m_gameScreen, &GameScreen::backToMainMenuRequested, this, &MainWindow::showMainMenu);
}

void MainWindow::showMainMenu()
{
    m_stackedWidget->setCurrentWidget(m_mainMenuScreen);
    m_mainMenuScreen->resetUI(); // <-- 【新增】调用UI重置函数

    AudioManager::instance()->playBgm(BgmType::MainMenu);
}

void MainWindow::showSettings()
{
    // 以模态方式显示设置对话框，它会阻塞主窗口直到对话框关闭
    m_settingsDialog->exec();
}

void MainWindow::startGame()
{
    // 这里是占位符，后续将切换到游戏界面
    AudioManager::instance()->playBgm(BgmType::GameScreen);
    qDebug() << "Start Game button clicked! Should switch to game screen.";
    // m_stackedWidget->setCurrentWidget(m_gameScreen);
    // // 让游戏界面开始自己的游戏循环
    // m_gameScreen->startGame();
    // 【核心修改】在显示游戏界面之前，先命令它进行一次彻底的重置
    m_gameScreen->restartGame();

    // 然后再切换到焕然一新的游戏界面
    m_stackedWidget->setCurrentWidget(m_gameScreen);
}
