#include "mainwindow.h"

#include <DApplication>
#include <QScreen>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

#define WIDTH 800
#define HEIGHT 600
#define APP_TITLE "deepin-theme-tool"

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setApplicationName(APP_TITLE);
    MainWindow w;
    w.titlebar()->setIcon(QIcon::fromTheme("application-x-theme"));
    w.setWindowTitle(APP_TITLE);
    w.setMinimumSize(WIDTH, HEIGHT);
    w.move((qApp->primaryScreen()->size().width() - WIDTH )/ 2, (qApp->primaryScreen()->size().height() - HEIGHT) / 2);
    w.show();
    return a.exec();
}
