#ifndef READERSTEP_H
#define READERSTEP_H

#include "Steps/step.h"
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
    void execute(QString data);
    void execute(cv::Mat data);
private:
    cv::Mat image;
signals:
    completed(QString);
};

#endif // READERSTEP_H
