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
};

#endif // VARIATIONBOUNDARYFINDERSTEPSTEP_H
