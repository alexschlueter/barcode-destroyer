#ifndef STEP_H
#define STEP_H

#include <QObject>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class Step : public QObject
{
    Q_OBJECT
public:
    Step();
public slots:
    virtual void execute(QString data);
    virtual void execute(cv::Mat data);

signals:
    completed(QString);
    completed(cv::Mat);
};

#endif // STEP_H
