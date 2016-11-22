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
            //TODO default barcode detection
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
    return true;
}
