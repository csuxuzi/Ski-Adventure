#include "ui/MainWindow.h"
#include "ui/MainMenuScreen.h"
#include "ui/SettingsDialog.h"
#include "audio/AudioManager.h"
#include <QStackedWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Ski Adventure");
    setFixedSize(800, 600);

    setupUI();
    createConnections();

    // 启动背景音乐
    AudioManager::instance()->playMainMenuMusic();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    m_mainMenuScreen = new MainMenuScreen(this);
    m_stackedWidget->addWidget(m_mainMenuScreen);

    m_settingsDialog = new SettingsDialog(this);

    // 默认显示主菜单
    showMainMenu();
}

void MainWindow::createConnections()
{
    connect(m_mainMenuScreen, &MainMenuScreen::startGameClicked, this, &MainWindow::startGame);
    connect(m_mainMenuScreen, &MainMenuScreen::settingsClicked, this, &MainWindow::showSettings);
}

void MainWindow::showMainMenu()
{
    m_stackedWidget->setCurrentWidget(m_mainMenuScreen);
}

void MainWindow::showSettings()
{
    // 以模态方式显示设置对话框，它会阻塞主窗口直到对话框关闭
    m_settingsDialog->exec();
}

void MainWindow::startGame()
{
    // 这里是占位符，后续将切换到游戏界面
    qDebug() << "Start Game button clicked! Should switch to game screen.";
    // 示例: m_stackedWidget->setCurrentWidget(m_gameScreen);
}