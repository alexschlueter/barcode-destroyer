#ifndef DETECTOR_H
#define DETECTOR_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QString>

class Detector
{
public:
    Detector(int index, QString path, int function = 0);

    void detect();
    QString result();
    bool isSuccessful();
private:
    int index;
    int function;
    QString path;
    QString code;
    bool successful;
    cv::Mat image;


    bool loadImage();
    void defaultDetector();
    void drawContourOnOriginalImage(std::vector< std::vector < cv::Point > > , uint contoursToDrawCount);
};

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 );

#endif // DETECTOR_H
