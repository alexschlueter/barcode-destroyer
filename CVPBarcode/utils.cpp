#include "utils.h"
#include <QWidget>

using namespace cv;

void drawRotatedRect(Mat& img, RotatedRect rect) {
    Point2f points[4];
    rect.points(points);
    for (int i = 0; i < 4; i++)
        line(img, points[i], points[(i+1)%4], {0, 0, 255}, 2);
    circle(img, points[0], 3, {255, 0, 0});
    circle(img, points[1], 3, {0, 255, 0});
    circle(img, points[2], 3, {0, 0, 255});
    circle(img, points[3], 3, {255, 255, 255});
}

void clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                delete widget;
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

cv::Mat rotateImage(cv::Mat img, int angle, int sizeFactor)
{
    cv::Mat out;
    cv::Point2i center = cv::Point2i(img.cols / 2, img.rows / 2);
    cv::Size size(img.cols * sizeFactor, img.rows * sizeFactor);

    cv::Mat rotation_mat = cv::getRotationMatrix2D(center, angle, 1);
    cv::warpAffine(img, out, rotation_mat, size);
    return out;
}
