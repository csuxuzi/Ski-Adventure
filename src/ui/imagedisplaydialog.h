#ifndef IMAGEDISPLAYDIALOG_H
#define IMAGEDISPLAYDIALOG_H

#include <QDialog>

class ImageDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数接收标题、图标路径和要显示的图片路径
    explicit ImageDisplayDialog(const QString &windowTitle, const QString &iconPath, const QString &imagePath, QWidget *parent = nullptr);
};

#endif // IMAGEDISPLAYDIALOG_H
