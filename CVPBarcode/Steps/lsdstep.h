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

public slots:
    void execute(void* data);

private:
    static constexpr float angleTol = 0.1f*180/CV_PI;
    static constexpr float lengthTol = 0.3f;
    static constexpr float projCenterTol = 0.1f;
    static constexpr float centerDistTol = 1.0f;
    static const std::array<float, 5> scanOffsets;

    cv::Mat gray;

    void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
    static bool linesMaybeInSameBarcode(const cv::Vec4f &line1, const cv::Vec4f &line2);
    cv::Point maxVariationDifferenceAlongLine(const cv::Point2f &start, const cv::Point2f &dir);
    template <class LineIt>
    static cv::Point extendBoundWithLines(const cv::Point &bound, const cv::Point2f &dir, float allowedDistance, LineIt linesBegin, LineIt linesEnd);
};

struct LSDResult {
    cv::Mat img;
    cv::Point leftBnd;
    cv::Point rightBnd;
    float height;

    LSDResult(cv::Mat _img, cv::Point _leftBnd, cv::Point _rightBnd, float _height) : img(_img), leftBnd(_leftBnd), rightBnd(_rightBnd), height(_height) {}
};

#endif // LSDSTEP_H
