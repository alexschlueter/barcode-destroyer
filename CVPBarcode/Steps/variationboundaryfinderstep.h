#ifndef VARIATIONBOUNDARYFINDERSTEPSTEP_H
#define VARIATIONBOUNDARYFINDERSTEPSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class VariationBoundaryFinderStep : public Step
{
    Q_OBJECT
public:
    VariationBoundaryFinderStep() {}

public slots:
    void execute(void* data);
private:
    static const std::array<float, 5> scanOffsets;

    cv::Point maxVariationDifferenceAlongLine(const cv::Mat &img, const cv::Point2f &start, const cv::Point2f &dir);
    template <class LineIt>
    static cv::Point extendBoundWithLines(const cv::Point &bound, const cv::Point2f &dir, float allowedDistance, LineIt linesBegin, LineIt linesEnd);
};

#endif // VARIATIONBOUNDARYFINDERSTEPSTEP_H
