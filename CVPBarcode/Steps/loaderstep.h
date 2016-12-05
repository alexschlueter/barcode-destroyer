#ifndef LOADERSTEP_H
#define LOADERSTEP_H

#include "Steps/step.h"
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class LoaderStep : public Step
{
    Q_OBJECT
public:
    LoaderStep();
public slots:
    void execute(QString data);
    void execute(cv::Mat data);

private:
    QString path;
    cv::Size getNewSize(cv::Mat & src, uint maxSize);
signals:
    completed(cv::Mat);
};

#endif // LOADERSTEP_H
