#ifndef UTILS_H
#define UTILS_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QLayout>

void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
void clearLayout(QLayout* layout, bool deleteWidgets = true);
cv::Mat rotateImage(cv::Mat img, int angle, int sizeFactor=1);


#endif // UTILS_H
