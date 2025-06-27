#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include "ui/MainWindow.h" // 注意路径

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 创建并显示启动画面
    QPixmap pixmap(":/assets/images/splash_screen.png");
    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents(); // 确保启动画面能被立即绘制出来

    // 2. 创建主窗口 (但先不显示)
    MainWindow w;

    // 3. 使用QTimer来模拟加载过程，并在2秒后关闭启动画面并显示主窗口
    QTimer::singleShot(2000, &splash, &QWidget::close);
    QTimer::singleShot(2000, &w, &QWidget::show);

    return a.exec();
}
