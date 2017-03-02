#ifndef SHOWSTEP_H
#define SHOWSTEP_H

#include "../Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class ShowStep : public Step
{
    Q_OBJECT
public:
    ShowStep(QString name = "Preview");
public slots:
    void execute(void* data);

private:
    cv::Mat image;
    QString name;
};

#endif // SHOWSTEP_H
