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
    std::vector<cv::Point> leftBnds;
    std::vector<cv::Point> rightBnds;
    float height;

    LocalizationResult(cv::Mat _img, std::vector<cv::Point> _leftBnds, std::vector<cv::Point> _rightBnds, float _height) : img(_img), leftBnds(_leftBnds), rightBnds(_rightBnds), height(_height) {}
};
struct BarcodeError : std::exception
{
    char text[1000];

    BarcodeError(char const* fmt, ...) __attribute__((format(printf,2,3))) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(text, sizeof text, fmt, ap);
        va_end(ap);
    }

    char const* what() const throw() { return text; }
};

#endif // STEP_H
