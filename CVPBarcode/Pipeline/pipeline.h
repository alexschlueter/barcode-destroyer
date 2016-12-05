#ifndef PIPELINE_H
#define PIPELINE_H

#include "Steps/step.h"
#include <QVector>
#include <QObject>

class Pipeline : public QObject
{
    Q_OBJECT
public:
    Pipeline();
    virtual void execute();
private:
    virtual void connectSteps(Step &step1, Step &step2);
};

#endif // PIPELINE_H
