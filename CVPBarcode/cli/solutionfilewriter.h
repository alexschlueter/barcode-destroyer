#ifndef SOLUTIONFILEWRITER_H
#define SOLUTIONFILEWRITER_H

#include "../Pipeline/pipeline.h"
#include <fstream>

#include <QObject>

class solutionFileWriter : public QObject
{
    Q_OBJECT
public:
    explicit solutionFileWriter(QObject *parent = 0, QString inputFile = "", QString outputFile = "" );

signals:

public slots:
};

#endif // SOLUTIONFILEWRITER_H
