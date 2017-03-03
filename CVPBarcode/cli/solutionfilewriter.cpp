#include "solutionfilewriter.h"

solutionFileWriter::solutionFileWriter(QObject *parent, QString inputFile, QString outputFile) : QObject(parent)
{
    auto pipe = getPipelines()[ "LSD + LSDBound + TemplateMatching" ]->create( inputFile );
    connect( pipe, &Pipeline::completed, this, [this, pipe, outputFile]( QString result ){

        std::ofstream outfile ( outputFile.toStdString() );
        outfile << result.toStdString() << std::endl;
        outfile.close();

        pipe->deleteLater();
    });
    QMetaObject::invokeMethod(pipe,"start",Qt::QueuedConnection);
}
