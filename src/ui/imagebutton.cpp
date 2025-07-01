#include "ui/ImageButton.h"
#include <QBitmap>
#include "audio/AudioManager.h"
// 不带缩放的构造函数 (它会调用下面的带缩放的构造函数，并传递默认缩放比例 1.0)
ImageButton::ImageButton(const QString &imagePath, QWidget *parent)
    : ImageButton(imagePath, 1.0, parent) // C++11 委托构造函数
{
    // 函数体为空，因为所有工作都委托给了另一个构造函数
}

ImageButton::ImageButton(const QString &imagePath, qreal scale,QWidget *parent)
    : QPushButton(parent), m_pixmap(imagePath)
{
    QPixmap originalPixmap(imagePath);

    // 根据传入的 scale 参数，生成一张缩放后的新图片
    // 如果 scale 是 1.0, 则图片大小不变
    m_pixmap = originalPixmap.scaled(originalPixmap.size() * scale,
         Qt::KeepAspectRatio,
         Qt::SmoothTransformation);


    // 设置按钮的基本属性
    setIcon(m_pixmap);
    setIconSize(m_pixmap.size());
    setFixedSize(m_pixmap.size());
    setStyleSheet("QPushButton { border: none; background: transparent; }");

    // 设置遮罩，让按钮的点击区域和图片的不透明区域完全一样
    setMask(m_pixmap.mask());

    // 连接点击信号，播放音效
    connect(this, &QPushButton::clicked, this, []() {
        AudioManager::instance()->playSoundEffect(SfxType::ButtonClick);
    });
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
