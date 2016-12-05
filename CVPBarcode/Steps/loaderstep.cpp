#include "loaderstep.h"

LoaderStep::LoaderStep()
{

}

void LoaderStep::execute(cv::Mat data){(void)data;}

void LoaderStep::execute(QString data){
    this->path = data;

    cv::Size scaledSize;
    cv::Mat color = cv::imread(data.toStdString());
    cv::Mat image;
    cv::cvtColor(color,image,CV_BGR2GRAY);
    scaledSize = getNewSize(image,500);
    cv::resize(image,image,scaledSize);
    emit completed(image);
}


cv::Size LoaderStep::getNewSize( cv::Mat & src, uint maxSize ) {
    uint newHeight, newWidth;
    float scaleX, scaleY;

    scaleX = (float) maxSize / (float) src.cols;
    scaleY = (float) maxSize / (float) src.rows;

    if ( scaleX < scaleY ) {
        newWidth = maxSize;
        newHeight = src.rows * scaleX;
    } else {
        newWidth = src.cols * scaleY;
        newHeight = maxSize;
    }

    return cv::Size(newWidth, newHeight);
}
