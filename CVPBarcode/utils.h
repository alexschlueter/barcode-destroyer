#ifndef UTILS_H
#define UTILS_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <QLayout>

void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
void clearLayout(QLayout* layout, bool deleteWidgets = true);
cv::Mat rotateImage(cv::Mat img, int angle, int sizeFactor=1);

template <class IT>
IT safeIncr(const IT &it, const IT &end, typename IT::difference_type i)
{
    return it + std::min(i, end-it);
}
template <class IT>
IT safeDecr(const IT &it, const IT &begin, typename IT::difference_type i)
{
    return it - std::min(i, it-begin);
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 3)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

#endif // UTILS_H
