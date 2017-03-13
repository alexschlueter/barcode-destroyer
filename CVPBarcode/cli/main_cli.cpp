#include <opencv2/core/core.hpp>
#include <iostream>
#include <QString>
//#include <QApplication>
//#include <QThread>

#include "solutionfilewriter.h"

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    //QThread pipethread;
    //pipethread.start();
    if ( argc != 3 ) {
        std::cout << "Incorrect number of arguments.\n usage: [inputFile] [outputFile]" << std::endl;
        return -1;
    }
    QString inputFile(argv[1]);
    QString outputFile(argv[2]);
    solutionFileWriter * s = new solutionFileWriter(inputFile, outputFile);
    //s->moveToThread(&pipethread);
    //while(s->waiting){
    //    pipethread.wait(100);
        //std::cout << "wait" << std::endl;
    //}
    //pipethread.quit();
    //pipethread.wait(100);
    //a.exec();
    return 0;
}

