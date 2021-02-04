#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication ETUPaint(argc, argv);
    MainWindow w;
    w.show();
    return ETUPaint.exec();
}
