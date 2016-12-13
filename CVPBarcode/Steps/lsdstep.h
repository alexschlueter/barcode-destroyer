#ifndef LSDSTEP_H
#define LSDSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class LSDStep : public Step
{
    Q_OBJECT
public:
    LSDStep() : scanOffsets{-0.3, -0.15, 0, 0.15, 0.3} {}

public slots:
    void execute(void* data);

private:
    const float angleTol = 0.1f*180/CV_PI;
    const float lengthTol = 0.3f;
    const float projCenterTol = 0.1f;
    const float centerDistTol = 1.0f;
    const std::array<float, 5> scanOffsets;

    cv::Mat gray;

    void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
    cv::Point maxVariationDifferenceAlongLine(const cv::Point2f &start, const cv::Point2f &dir);
};

struct LSDResult {
    cv::Mat img;
    cv::Point leftBnd;
    cv::Point rightBnd;
    float height;

    LSDResult(cv::Mat _img, cv::Point _leftBnd, cv::Point _rightBnd, float _height) : img(_img), leftBnd(_leftBnd), rightBnd(_rightBnd), height(_height) {}
};

#endif // LSDSTEP_H
