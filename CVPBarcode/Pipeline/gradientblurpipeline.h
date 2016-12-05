#ifndef GRADIENTBLURPIPELINE_H
#define GRADIENTBLURPIPELINE_H

#include "Pipeline/pipeline.h"

#include "Steps/loaderstep.h"
#include "Steps/showstep.h"
#include "Steps/readerstep.h"

#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class GradientBlurPipeline : public Pipeline
{
public:
    GradientBlurPipeline();
    void execute();

private:
    void connectSteps(Step &step1, Step &step2);
    QVector<Step> steps;
};

#endif // GRADIENTBLURPIPELINE_H
