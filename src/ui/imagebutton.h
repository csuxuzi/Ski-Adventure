#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QPushButton>
#include <QPixmap>
class ImageButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ImageButton(const QString &imagePath, QWidget *parent = nullptr);

    // 设置当鼠标悬停时是否改变大小
    void setHoverEnabled(bool enabled);
    void updateIcon(const QPixmap &pixmap);
protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPixmap m_pixmap;
    bool m_hoverEnabled = false;
};

#endif // IMAGEBUTTON_H
