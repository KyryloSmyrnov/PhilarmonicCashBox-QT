#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication a(argc, argv);
    MainWindow w;
    w.resize(1920,1080);
    w.show();
    return a.exec();
}
