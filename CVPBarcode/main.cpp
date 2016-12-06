#include "mainwindow.h"
#include <QApplication>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    cout << CV_VERSION << endl;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
