#include "ImageDisplayDialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>

ImageDisplayDialog::ImageDisplayDialog(const QString &windowTitle, const QString &iconPath, const QString &imagePath, QWidget *parent)
    : QDialog(parent)
{
    // 1. 设置窗口的标题和图标
    setWindowTitle(windowTitle);
    setWindowIcon(QIcon(iconPath));

    // 2. 创建一个 QLabel 用于显示图片
    QLabel* imageLabel = new QLabel(this);
    QPixmap pixmap(imagePath);

    // 3. 加载图片并根据图片大小调整窗口尺寸
    if (!pixmap.isNull()) {
        imageLabel->setPixmap(pixmap);
        setFixedSize(pixmap.size()); // 让窗口和图片一样大
    } else {
        // 如果图片加载失败，显示提示文字
        imageLabel->setText("图片加载失败！");
        setFixedSize(400, 200);
    }

    // 4. 设置布局，让图片填充整个窗口
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(imageLabel);
    layout->setContentsMargins(0, 0, 0, 0); // 无边距
    setLayout(layout);
}
