#ifndef STEP_H
#define STEP_H

#include <QObject>
#include <QThread>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class Step : public QObject
{
    Q_OBJECT

protected:
    bool visualize;

public slots:
    virtual void execute(void *data);

signals:
    void completed(void*);
    void showImage(const std::string &, const cv::Mat &);
};

struct LocalizationResult {
    cv::Mat img;
    cv::Point leftBnd;
    cv::Point rightBnd;
    float height;

    LocalizationResult(cv::Mat _img, cv::Point _leftBnd, cv::Point _rightBnd, float _height) : img(_img), leftBnd(_leftBnd), rightBnd(_rightBnd), height(_height) {}
};

#endif // STEP_H
