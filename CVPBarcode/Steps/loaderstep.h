#ifndef LOADERSTEP_H
#define LOADERSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class LoaderStep : public Step
{
    Q_OBJECT
public:
    LoaderStep();
public slots:
    void execute(void* data);

private:
    cv::Size getNewSize(cv::Mat & src, uint maxSize);

};

#endif // LOADERSTEP_H
