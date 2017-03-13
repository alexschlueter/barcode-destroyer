#ifndef LSDBOUNDARYFINDERSTEP_H
#define LSDBOUNDARYFINDERSTEP_H

#include "step.h"

class LSDBoundaryFinderStep : public Step
{
    Q_OBJECT
public:

    template <class LineIt>
    std::vector<cv::Point> extendBoundWithLines(const cv::Point &bound, const cv::Point2f &dir, float allowedDistance, LineIt linesBegin, LineIt linesEnd);
public slots:
    void execute(void* data);
};

#endif // LSDBOUNDARYFINDERSTEP_H
