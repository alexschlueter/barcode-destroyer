#ifndef DETECTOR_H
#define DETECTOR_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QString>
#include <limits>
#include <array>

class Detector
{
public:
    Detector(int index, QString path, int function = 1);

    void detect();
    QString result();
    bool isSuccessful();
private:
    int index;
    int function;
    QString path;
    QString code;
    bool successful;
    cv::Mat color;
    cv::Mat gray;


    bool loadImage();
    cv::Size getNewSize(cv::Mat inputImage, uint maxSize );
    void defaultDetector();
    void drawRotatedRect(cv::Mat& img, cv::RotatedRect rect);
    cv::Point maxVariationDifferenceAlongLine(const cv::Point2f &start, const cv::Point2f &dir);
    void lineSegmentDetector();
    void drawContourOnOriginalImage(std::vector< std::vector < cv::Point > > , uint contoursToDrawCount);
};

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 );

#endif // DETECTOR_H
