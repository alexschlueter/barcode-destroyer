#include "loaderstep.h"

LoaderStep::LoaderStep()
{

}

void LoaderStep::execute(void *data){
    this->path = *static_cast<QString*>(data);
    cv::Size scaledSize;
    cv::Mat color = cv::imread(path.toStdString());
    cv::Mat *image = new cv::Mat;
    cv::cvtColor(color,*image,CV_BGR2GRAY);
    //scaledSize = getNewSize(*image,500);
    //cv::resize(*image,*image,scaledSize);
    emit completed((void*)image);
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
