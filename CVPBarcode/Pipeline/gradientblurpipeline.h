#ifndef GRADIENTBLURPIPELINE_H
#define GRADIENTBLURPIPELINE_H

#include "../Pipeline/pipeline.h"

#include "../Steps/loaderstep.h"
#include "../Steps/showstep.h"
#include "../Steps/readerstep.h"
#include "../Steps/gradientblurstep.h"

#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class GradientBlurPipeline : public Pipeline
{
public:
    GradientBlurPipeline(QString path):Pipeline(path){}
    void execute(void* data);
};

#endif // GRADIENTBLURPIPELINE_H
