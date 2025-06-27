#include "ui/ImageButton.h"
#include <QBitmap>

ImageButton::ImageButton(const QString &imagePath, QWidget *parent)
    : QPushButton(parent), m_pixmap(imagePath)
{
    // 设置按钮的基本属性
    setIcon(m_pixmap);
    setIconSize(m_pixmap.size());
    setFixedSize(m_pixmap.size());
    setStyleSheet("QPushButton { border: none; background: transparent; }");

    // 关键：设置遮罩，让按钮的点击区域和图片的不透明区域完全一样
    setMask(m_pixmap.mask());
}
void ImageButton::updateIcon(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    setIcon(m_pixmap);
    setIconSize(m_pixmap.size());
    setFixedSize(m_pixmap.size());
    setMask(m_pixmap.mask());
}
void ImageButton::setHoverEnabled(bool enabled)
{
    m_hoverEnabled = enabled;
}

void ImageButton::enterEvent(QEnterEvent *event)
{
    if (m_hoverEnabled) {
        // 鼠标进入时放大图标
        setIconSize(m_pixmap.size() * 1.1);
    }
    QPushButton::enterEvent(event);
}

void ImageButton::leaveEvent(QEvent *event)
{
    if (m_hoverEnabled) {
        // 鼠标离开时恢复原大小
        setIconSize(m_pixmap.size());
    }
    QPushButton::leaveEvent(event);
}
