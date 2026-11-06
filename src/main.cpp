#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("SimpleTextEditor");
    QApplication::setOrganizationName("DGUT");

    MainWindow w;
    w.show();
    return app.exec();
}
