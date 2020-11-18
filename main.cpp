#include "scanner_gui.h"
#include <QtWidgets>
#include <QtMultimediaWidgets>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    scanner_gui w;
    w.setWindowState(Qt::WindowMaximized);
    a.setOrganizationName("SDU Søndeborg");
    a.setOrganizationDomain("EMC LAB");
    a.setApplicationName("Near Field Scanner");
    w.show();
    w.init();
    return a.exec();
}
