#ifndef LSDSTEP_H
#define LSDSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/*
 * paper "Low-Computation Egocentric Barcode Detector for the Blind"
 * uses LineSegmentDetector provided by opencv-3.*
 * LineSegmentDetector is described here: http://www.ipol.im/pub/art/2012/gjmr-lsd/
*/
class LSDStep : public Step
{
    Q_OBJECT

public:
    static bool linesMaybeInSameBarcode(const cv::Vec4f &line1, const cv::Vec4f &line2);

public slots:
    void execute(void* data);

private:
    static constexpr float angleTol = 0.1f*180/CV_PI;
    static constexpr float lengthTol = 0.3f;
    static constexpr float projCenterTol = 0.1f;
    static constexpr float centerDistTol = 1.0f;

    cv::Mat gray;
};

struct LSDResult
{
    cv::Mat gray;
    // start and end points of best line
    cv::Vec4f bestLine;

    std::vector<cv::Vec4f> lines;

    LSDResult(cv::Mat _gray, cv::Vec4f _bestLine, std::vector<cv::Vec4f> _lines)
        : gray(std::move(_gray)), bestLine(std::move(_bestLine)), lines(std::move(_lines))
    {}
};

#endif // LSDSTEP_H
