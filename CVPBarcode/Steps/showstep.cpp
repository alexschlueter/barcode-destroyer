#include "showstep.h"

ShowStep::ShowStep()
{

}

void ShowStep::execute(QString data){(void)data;}

void ShowStep::execute(cv::Mat data){
    this->image = data;
    cv::imshow("Preview", image);
    emit completed(image);
}
