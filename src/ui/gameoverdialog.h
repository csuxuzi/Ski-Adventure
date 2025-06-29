#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <QDialog>

class ImageButton;

class GameOverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameOverDialog(QWidget *parent = nullptr);

signals:
    // 定义清晰的信号，告诉外界用户的选择
    void restartClicked();
    void exitClicked();

private:
    void setupUI();

    ImageButton* m_restartButton;
    ImageButton* m_exitButton;
};

#endif // GAMEOVERDIALOG_H
