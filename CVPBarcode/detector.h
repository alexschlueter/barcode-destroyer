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
    cv::Size getNewSize(cv::Mat inputImage, uint maxSize );
};

#endif // DETECTOR_H
