#include <opencv2/core/core.hpp>
#include <iostream>
#include <QString>

#include "solutionfilewriter.h"

int main(int argc, char *argv[])
{
    if ( argc != 3 ) {
        std::cout << "Incorrect number of arguments." << std::endl;
        return -1;
    }
    QString inputFile(argv[1]);
    QString outputFile(argv[2]);
    new solutionFileWriter(0, inputFile, outputFile);

    return 0;
}

