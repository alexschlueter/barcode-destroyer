#include "gradientblurstep.h"
#include "utils.h"
#include <iostream>

using namespace std;

GradientBlurStep::GradientBlurStep()
{

}

void GradientBlurStep::execute(void *data){
    this->image = static_cast<cv::Mat*>(data)->clone();

    cv::Mat gradX, gradY, grad, gradAbs, blur, threshold, kernel, closed, eroded, dilated;
    std::vector< std::vector< cv::Point > > contours;
    cv::RotatedRect boundingBox;

    cv::Sobel( image, gradX, CV_32F, 1, 0, -1 );
    cv::Sobel( image, gradY, CV_32F, 0, 1, -1 );

    cv::subtract(gradX, gradY, grad);
    cv::convertScaleAbs(grad, gradAbs );
    emit showImage("AbsGradient", gradAbs);

    cv::blur( gradAbs, blur, cv::Point(9, 9) );
    double mean = cv::mean(blur)[0];
    cv::threshold( blur, threshold, 0.5*(255+mean), 255, cv::THRESH_BINARY );
    //cv::threshold( blur, threshold, 225, 255, cv::THRESH_BINARY );
    emit showImage("Blur + Threshold", threshold);

    kernel = cv::getStructuringElement( cv::MORPH_RECT, cv::Point(21, 7) );
    cv::morphologyEx( threshold, closed, cv::MORPH_CLOSE, kernel );
    emit showImage("Closed", closed);

    cv::erode( closed, eroded, cv::Mat(), cv::Point(-1, -1), 4 );
    emit showImage("Eroded", eroded);
    cv::dilate( eroded, dilated, cv::Mat(), cv::Point(-1, -1), 4 );
    emit showImage("Dilated", dilated);

    cv::findContours( eroded.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
    LocalizationResult *res;

    if (contours.empty()) {
        std::cout << "GradientBlurStep: no contours found" << std::endl;
        res = new LocalizationResult(*static_cast<cv::Mat*>(data), {0, image.rows/2}, {image.cols-1, image.rows/2}, image.rows);
    } else {
        std::sort( contours.begin(), contours.end(), compareContourAreas );

        cv::Mat color;
        cv::cvtColor(image, color, cv::COLOR_GRAY2BGR);
        drawContours(color, contours, 1);


        cv::RotatedRect rect = cv::minAreaRect(contours[0]);
        drawRotatedRect(color, rect);
        emit showImage("Largest Contour + Bounding Box", color);

        float height;// = std::min(rect.size.width, rect.size.height);
        cv::Point leftBoundary, rightBoundary;
        cv::Point2f points[4];
        rect.points(points);

        if (rect.size.height < rect.size.width) {
            height = rect.size.height;
            leftBoundary = 0.5f*(points[0]+points[1]);
            rightBoundary = 0.5f*(points[2]+points[3]);
        } else {
            height = rect.size.width;
            leftBoundary = 0.5f*(points[0]+points[3]);
            rightBoundary = 0.5f*(points[1]+points[2]);
        }

        res = new LocalizationResult(*static_cast<cv::Mat*>(data), leftBoundary, rightBoundary, height);
    }
    delete static_cast<cv::Mat*>(data);
    emit completed((void*)res);
}


bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}

void GradientBlurStep::drawContours(cv::Mat &img, std::vector< std::vector< cv::Point > > contours, uint contoursToDrawCount ) {
    int count = ( contoursToDrawCount < contours.size() ) ? contoursToDrawCount : contours.size();
    for ( int i = 0; i < count; ++i ) {
        cv::drawContours( img, contours, i, cv::Scalar(0, 255, 0), 2, 8 );
    }
}
