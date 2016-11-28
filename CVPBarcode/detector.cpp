#include "detector.h"

Detector::Detector(int index, QString path, int function)
{
    this->index = index;
    this->path = path;
    this->code = "";
    this->successful = false;
    this->function = function;

}

void Detector::detect(){
    if(loadImage()){
        cv::imshow("test",image);
        switch (function) {
        case 1:
            //TODO alternative functions
            break;
        default:
            defaultDetector();
            break;
        }
        //TODO transform barcode

        //TODO readBarcode

        //successful = true;
        //code = "*somecode*";
    } else {
        successful = false;
    }
}

bool Detector::isSuccessful(){
    return successful;
}

QString Detector::result(){
    return this->code;
}

bool Detector::loadImage(){
    cv::Mat color = cv::imread(path.toStdString());
    //Turn image to Grayscale
    cv::cvtColor(color,image,CV_BGR2GRAY);
    //TODO maybe resize the image?
    // Kjeld: Agree - this would very likely even improve the results of Gradient + Blur
    return true;
}

void Detector::defaultDetector() {
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
    drawContourOnOriginalImage( contours, 1 );


    cv::imshow("Detected Code", image);
}

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}

void Detector::drawContourOnOriginalImage( std::vector< std::vector< cv::Point > > contours, uint contoursToDrawCount ) {
    int count = ( contoursToDrawCount < contours.size() ) ? contoursToDrawCount : contours.size();
    for ( int i = 0; i < count; ++i ) {
        cv::drawContours( image, contours, i, cv::Scalar(0, 255, 0), 2, 8 );
    }
}
