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
public:
    LSDPipeline(QString path) : Pipeline(path) {}
    void execute(void* data);
};

#endif // LSDPIPELINE_H
