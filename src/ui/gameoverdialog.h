#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <QDialog>
class QLabel;
class ImageButton;

class GameOverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameOverDialog(QWidget *parent = nullptr);
    // 公开的函数，用于从外部设置分数
    void setScore(quint64 score);

signals:
    // 定义清晰的信号，告诉外界用户的选择
    void restartClicked();
    void exitClicked();

private:
    void setupUI();

    ImageButton* m_restartButton;
    ImageButton* m_exitButton;
    QLabel* m_titleLabel;      // 将 titleLabel 提升为成员变量
    QLabel* m_scoreLabel;      // 用于显示分数的标签
};

#endif // GAMEOVERDIALOG_H
