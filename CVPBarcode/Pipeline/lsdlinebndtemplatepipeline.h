#ifndef LSDLINEBNDTEMPLATEPIPELINE_H
#define LSDLINEBNDTEMPLATEPIPELINE_H

#include "../Pipeline/pipeline.h"
#include <QString>

class LSDLineBndTemplatePipeline : public Pipeline
{
public:
    LSDLineBndTemplatePipeline(QString path) : Pipeline(path) {}
    void execute(void* data);
};

#endif // LSDLINEBNDTEMPLATEPIPELINE_H
