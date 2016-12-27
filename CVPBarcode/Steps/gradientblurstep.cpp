#include "gradientblurstep.h"

GradientBlurStep::GradientBlurStep()
{

}

void GradientBlurStep::execute(void *data){
    this->image = *static_cast<cv::Mat*>(data);

    cv::Mat gradX, gradY, grad, gradAbs, blur, threshold, kernel, closed, eroded, dilated;
    std::vector< std::vector< cv::Point > > contours;
    cv::RotatedRect boundingBox;

    cv::Sobel( image, gradX, CV_32F, 1, 0, -1 );
    cv::Sobel( image, gradY, CV_32F, 0, 1, -1 );

    cv::subtract(gradX, gradY, grad);
    cv::convertScaleAbs(grad, gradAbs );

    cv::blur( gradAbs, blur, cv::Point(9, 9) );
    cv::threshold( blur, threshold, 225, 255, cv::THRESH_BINARY );

    kernel = cv::getStructuringElement( cv::MORPH_RECT, cv::Point(21, 7) );
    cv::morphologyEx( threshold, closed, cv::MORPH_CLOSE, kernel );

    cv::erode( closed, eroded, cv::Mat(), cv::Point(-1, -1), 4 );
    cv::dilate( eroded, dilated, cv::Mat(), cv::Point(-1, -1), 4 );

    cv::findContours( eroded.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
    std::sort( contours.begin(), contours.end(), compareContourAreas );
    drawContourOnOriginalImage(contours,1);

    emit showImage("GradientBlur", image);
    emit completed((void*)&image);
}


bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}

void GradientBlurStep::drawContourOnOriginalImage( std::vector< std::vector< cv::Point > > contours, uint contoursToDrawCount ) {
    int count = ( contoursToDrawCount < contours.size() ) ? contoursToDrawCount : contours.size();
    for ( int i = 0; i < count; ++i ) {
        cv::drawContours( image, contours, i, cv::Scalar(0, 255, 0), 2, 8 );
    }
}
