#include "utils.h"

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
