#ifndef SHOWSTEP_H
#define SHOWSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class ShowStep : public Step
{
    Q_OBJECT
public:
    ShowStep();
public slots:
    void execute(QString data);
    void execute(cv::Mat data);

private:
    cv::Mat image;

signals:
    completed(cv::Mat);
};

#endif // SHOWSTEP_H
