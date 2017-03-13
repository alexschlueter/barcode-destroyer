#ifndef GRADIENTBLURSTEP_H
#define GRADIENTBLURSTEP_H

#include "../Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class GradientBlurStep : public Step
{
    Q_OBJECT
public:
    GradientBlurStep();

public slots:
    void execute(void* data);

private:
    cv::Mat image;
    void drawContours(cv::Mat &img, std::vector< std::vector < cv::Point > > , uint contoursToDrawCount);

};
bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 );

#endif // GRADIENTBLURSTEP_H
