#include "mainwindow.h"
#include <opencv2/core/core.hpp>
#include <QApplication>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    cout << "opencv version = " << CV_VERSION << endl;
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("BarcodeDestroyerInc");
    QCoreApplication::setApplicationName("CVPBarcode");
    MainWindow w;
    w.show();

    return a.exec();
}
