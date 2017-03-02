#ifndef READERSTEP_H
#define READERSTEP_H

#include "../Steps/step.h"
#include "QString"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class ReaderStep : public Step
{
    Q_OBJECT
public:
    ReaderStep();
public slots:
    void execute(void* data);
private:
    cv::Mat image;
};

#endif // READERSTEP_H
