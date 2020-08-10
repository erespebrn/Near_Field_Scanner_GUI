#include "scanner_gui.h"
#include <QtWidgets>
#include <QtMultimediaWidgets>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    scanner_gui w;
    w.setWindowState(Qt::WindowMaximized);
    //w.setWindowState(Qt::WindowFullScreen);
    w.show();
    return a.exec();
}
