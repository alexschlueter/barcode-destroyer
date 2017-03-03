#ifndef SOLUTIONFILEWRITER_H
#define SOLUTIONFILEWRITER_H

#include <QObject>

class solutionFileWriter : public QObject
{
    Q_OBJECT
public:
    explicit solutionFileWriter(QObject *parent = 0);

signals:

public slots:
};

#endif // SOLUTIONFILEWRITER_H