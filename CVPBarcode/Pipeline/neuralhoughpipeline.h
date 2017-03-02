#ifndef NEURALHOUGHPIPELINE_H
#define NEURALHOUGHPIPELINE_H

#include "../Pipeline/pipeline.h"

#include "../Steps/loaderstep.h"
#include "../Steps/readerstep.h"
#include "../Steps/neuralhoughstep.h"

#include <QString>

class NeuralHoughPipeline : public Pipeline
{
public:
    NeuralHoughPipeline(QString path) : Pipeline(path) {}
    void execute(void* data);
};

#endif // NEURALHOUGHPIPELINE_H
