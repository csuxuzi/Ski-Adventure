#ifndef PAUSEDIALOG_H
#define PAUSEDIALOG_H

#include <QDialog>

class ImageButton;

class PauseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PauseDialog(QWidget *parent = nullptr);

signals:
    // 定义三个清晰的信号，告诉外界用户点击了哪个按钮
    void resumeClicked();
    void restartClicked();
    void exitClicked();

private:
    void setupUI();

    ImageButton* m_resumeButton;
    ImageButton* m_restartButton;
    ImageButton* m_exitButton;
};

#endif // PAUSEDIALOG_H
