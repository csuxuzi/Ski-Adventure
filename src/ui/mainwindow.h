#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class MainMenuScreen;
class SettingsDialog;
class GameScreen;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showMainMenu();
    void showSettings();
    void startGame();

private:
    void setupUI();
    void createConnections();

    QStackedWidget* m_stackedWidget;
    MainMenuScreen* m_mainMenuScreen;
    SettingsDialog* m_settingsDialog;
    GameScreen* m_gameScreen;
};

#endif // MAINWINDOW_H
