#include "solutionfilewriter.h"
#include <QDebug>

solutionFileWriter::solutionFileWriter(QString inputFile, QString outputFile) : QObject()
{
    auto pipe = getPipelines()[ "LSD + LSDBound + TemplateMatching" ]->create( inputFile );
    connect( pipe, &Pipeline::completed, this, [this, pipe, outputFile]( QString result ){

        std::ofstream outfile ( outputFile.toStdString() );
        if(result!="fail")
            outfile << result.toStdString() << std::endl;
        outfile.close();

        pipe->deleteLater();
    });
    pipe->start();

}
