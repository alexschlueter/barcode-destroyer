#ifndef LSDMUENSTERTEMPLATEPIPELINE_H
#define LSDMUENSTERTEMPLATEPIPELINE_H


#include "Pipeline/pipeline.h"
#include <QString>

class LSDMuensterTemplatePipeline : public Pipeline
{
public:
    LSDMuensterTemplatePipeline(QString path) : Pipeline(path) {}
    void execute(void* data);
};

#endif // LSDMUENSTERTEMPLATEPIPELINE_H
