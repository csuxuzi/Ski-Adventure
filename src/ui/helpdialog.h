#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QList>
#include <QPixmap>

class ImageButton;
class QLabel;

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);

private slots:
    void showNextImage();
    void showPreviousImage();

private:
    void setupUI();
    void loadHelpImages();
    void updateUI(); // 一个核心函数，用于更新图片和箭头状态

    ImageButton* m_leftArrow;
    ImageButton* m_rightArrow;
    QLabel* m_imageLabel; // 用于显示帮助图片

    QList<QPixmap> m_helpImages; // 存放所有帮助图片
    int m_currentIndex;          // 记录当前显示的图片索引
};

#endif // HELPDIALOG_H
