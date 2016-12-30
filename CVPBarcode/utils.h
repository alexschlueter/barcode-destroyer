#ifndef UTILS_H
#define UTILS_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QLayout>

void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
void clearLayout(QLayout* layout, bool deleteWidgets = true);

#endif // UTILS_H
