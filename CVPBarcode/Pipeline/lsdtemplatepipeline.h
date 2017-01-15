#ifndef LSDPIPELINE_H
#define LSDPIPELINE_H

#include "Pipeline/pipeline.h"
#include <QString>

class LSDTemplatePipeline : public Pipeline
{
public:
    LSDTemplatePipeline(QString path) : Pipeline(path) {}
    void execute(void* data);
};

#endif // LSDPIPELINE_H
