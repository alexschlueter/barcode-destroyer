#ifndef LSDPIPELINE_H
#define LSDPIPELINE_H

#include "Pipeline/pipeline.h"

#include "Steps/loaderstep.h"
#include "Steps/showstep.h"
#include "Steps/readerstep.h"
#include "Steps/lsdstep.h"

#include <QString>

class LSDPipeline : public Pipeline
{
    bool visualize;
public:
    LSDPipeline(QString path, bool _visualize=false) : Pipeline(path), visualize(_visualize) {}
    void execute(void* data);
};

#endif // LSDPIPELINE_H
